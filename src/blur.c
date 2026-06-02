#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

# define TAM_KERNEL 9 // (3 = 3x3) (5 = 5x5) (9 = 9x9)
# define TAM_PADDING ((TAM_KERNEL - 1) / 2) // Calcula o padding necessário para o kernel

// Struct com tamanho suficiente para acomodar um pixel RGB
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

// Struct maior para acomodar a soma de pixels sem overflow
typedef struct {
    unsigned int r;
    unsigned int g;
    unsigned int b;
} SomaPixel;

/*
 * Analisa o nome do arquivo de entrada e anexa a string "-blur.ppm"
 * antes da extensão para gerar o arquivo de saída esperado pelo backend.
 */
void gerar_nome_arquivo_saida(const char *input_name, char *output_name) {
    const char *dot = strrchr(input_name, '.');
    
    if (dot != NULL) {
        size_t base_len = dot - input_name;
        strncpy(output_name, input_name, base_len);
        output_name[base_len] = '\0';
        strcat(output_name, "-blur.ppm");
    } else {
        strcpy(output_name, input_name);
        strcat(output_name, "-blur.ppm");
    }
}

void ignorar_comentarios(FILE *arquivo) {
    int c;
    
    // Lê caracteres do arquivo repetidamente
    while ((c = fgetc(arquivo)) != EOF) {
        // Se o caractere for um espaço, pula para a próxima iteração
        if (isspace(c)) {
            continue;
        }
        
        // Se for #, lê até a quebra de linha
        if (c == '#') {
            while ((c = fgetc(arquivo)) != '\n' && c != EOF);
        } else { // Se não for, volta o cursor do arquivo em um caractere
            ungetc(c, arquivo);
            break;
        }
    }
}

// Desaloca em cascata a memoria de uma matriz bidimensional de Pixels.
void desalocar_imagem(Pixel **imagem, size_t altura_total) {
    if (imagem == NULL) {
        return;
    }

    // Libera a memoria alocada para as colunas de cada linha
    for (size_t i = 0; i < altura_total; i++) {
        if (imagem[i] != NULL) {
            free(imagem[i]);
        }
    }

    // Libera o vetor de ponteiros principal
    free(imagem);
}

int main(int argc, char *argv[]) {
    // Validação estrita da linha de comandos esperada pelo Node.js
    if (argc != 2) {
        fprintf(stderr, "Erro de sintaxe. Uso esperado: %s <caminho_imagem.ppm>\n", argv[0]);
        return 1;
    }

    const char *nome_arq_entrada = argv[1];
    char nome_arq_saida[256];

    // Metadados da imagem
    char magic_number[3];
    size_t altura_original;
    size_t largura_original;
    int max_cor;
    
    gerar_nome_arquivo_saida(nome_arq_entrada, nome_arq_saida);

    // Abertura explícita em modo binário ("rb" e "wb") para preservar 
    // a integridade de ficheiros de imagem (crucial para PPM formato P6)
    FILE *arq_entrada = fopen(nome_arq_entrada, "rb");
    if (!arq_entrada) {
        fprintf(stderr, "Erro ao abrir o arquivo de origem: %s\n", nome_arq_entrada);
        return 1;
    }   
    
    // Abrindo arquivo de saída
    FILE *arq_saida = fopen(nome_arq_saida, "wb");
    if (!arq_saida) {
        fprintf(stderr, "Erro ao criar o arquivo de destino: %s\n", nome_arq_saida);
        fclose(arq_entrada);
        return 1;
    }

    // Verificando se o arquivo é PPM P6, analisando os dois primeiros bytes
    fgets(magic_number, sizeof(magic_number), arq_entrada);
    if (magic_number[0] != 'P' || magic_number[1] != '6') {
        fprintf(stderr, "Erro: o arquivo não é PPM P6\n");
        return 1;
    } 

    // Lendo a largura e altura da imagem
    ignorar_comentarios(arq_entrada);
    fscanf(arq_entrada, "%zu %zu", &largura_original, &altura_original);

    // Lendo o valor máximo de cor
    ignorar_comentarios(arq_entrada);
    fscanf(arq_entrada, "%d", &max_cor);

    /* Preparando a transição TEXTO -> BINÁRIO: 
        * Lê o caractere em branco logo após o max_cor.
        * Se for um '\r' (Windows CRLF), consome o '\n' subsequente
    */
    int char_transicao = fgetc(arq_entrada);
    if (char_transicao == '\r') {
        int proximo = fgetc(arq_entrada);
        // Se por alguma anomalia o próximo não for \n, devolvemos para o buffer
        if (proximo != '\n') {
            ungetc(proximo, arq_entrada);
        }
    }
    
    if (max_cor > 255) {
    fprintf(stderr, "Erro: O programa suporta apenas imagens com valor maximo de cor ate 255 (8-bit).\n");
    fclose(arq_entrada);
    fclose(arq_saida);
    return 1; 
    }

    // Início da lógica principal
    // Definindo altura e largura total (orignal + paddding) 
    size_t altura_total = altura_original + (2 * TAM_PADDING);
    size_t largura_total = largura_original + (2 * TAM_PADDING);

    // Alocando memória para as linhas da imagem + padding
    Pixel **imagem_lida = malloc(sizeof(Pixel*) * altura_total);
    if (imagem_lida == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        fclose(arq_entrada);
        fclose(arq_saida);
        return 1;
    }

    // Alocando para as colunas + padding
    for (size_t i = 0; i < altura_total; i++) {
        imagem_lida[i] = malloc(sizeof(Pixel) * largura_total);

        if (imagem_lida[i] == NULL) {
            fprintf(stderr, "Erro ao alocar memória\n");
            desalocar_imagem(imagem_lida, i);
            fclose(arq_entrada);
            fclose(arq_saida);
            return 1;
        }
    }

    // Lendo os pixels da imagem, linha por linha
    for (size_t i = TAM_PADDING; i < altura_original + TAM_PADDING; i++) {
        // Aponta para a primeira posição útil da linha (pulando o padding esquerdo)
        Pixel *inicio_linha_util = &imagem_lida[i][TAM_PADDING];
        
        size_t lidos = fread(inicio_linha_util, sizeof(Pixel), largura_original, arq_entrada);

        if (lidos < largura_original) {
            fprintf(stderr, "Erro crítico: Fim de arquivo inesperado ou corrupção ao ler a linha %zu.\n", i);
            desalocar_imagem(imagem_lida, altura_total);
            fclose(arq_entrada);
            fclose(arq_saida);
            return 1;
        }
    }

    // Lógica de Edge Replication (extrapola as cores das bordas da imagem original para preencher a margem de padding)
    for (size_t i = 0; i < altura_total; i++) {
        // Restringe o índice vertical para a linha real mais próxima
        size_t orig_i = i;
        if (orig_i < TAM_PADDING) orig_i = TAM_PADDING;
        if (orig_i >= altura_original + TAM_PADDING) orig_i = altura_original + TAM_PADDING - 1;

        for (size_t j = 0; j < largura_total; j++) {
            // Se estiver dentro do miolo original da imagem, pula a replicação
            if (i >= TAM_PADDING && i < altura_original + TAM_PADDING &&
                j >= TAM_PADDING && j < largura_original + TAM_PADDING) {
                continue;
            }
            
            // Restringe o índice horizontal para a coluna real mais próxima
            size_t orig_j = j;
            if (orig_j < TAM_PADDING) orig_j = TAM_PADDING;
            if (orig_j >= largura_original + TAM_PADDING) orig_j = largura_original + TAM_PADDING - 1;

            // Replica a cor do píxel útil mais próximo na posição do padding
            imagem_lida[i][j] = imagem_lida[orig_i][orig_j];
        }
    }

    // Alocando memória para as linhas da imagem final
    Pixel **imagem_final = malloc(sizeof(Pixel*) * altura_original);
    if (imagem_final == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        desalocar_imagem(imagem_lida, altura_total);
        fclose(arq_entrada);
        fclose(arq_saida);
        return 1;
    }

    // Alocando para as colunas da imagem final
    for (size_t i = 0; i < altura_original; i++) {
        imagem_final[i] = malloc(sizeof(Pixel) * largura_original);

        if (imagem_final[i] == NULL) {
            fprintf(stderr, "Erro ao alocar memória\n");
            desalocar_imagem(imagem_lida, altura_total);
            desalocar_imagem(imagem_final, i);
            fclose(arq_entrada);
            fclose(arq_saida);
            return 1;
        }
    }

    /* Aplicando o algoritmo de convolução: 
        varrendo a imagem inicial, deslizando o kernel por cada pixel dela, efetuando a média aritmética dos valores
        de cor do píxel central e dos seus vizinhos (que, nas extremidades, farão a leitura dos píxeis replicados no 
        padding) e gravando o resultado na posição correspondente do buffer de escrita.
    */ 
    for (size_t i = TAM_PADDING; i < altura_original + TAM_PADDING; i++) {
        for (size_t j = TAM_PADDING; j < largura_original + TAM_PADDING; j++) {
            SomaPixel soma = {0, 0, 0};
            
            /* * Varre a vizinhanca utilizando deslocamentos relativos com sinal (int),
             * permitindo caminhar para trás e para frente ao redor do píxel central.
             */
            for (int k = -(int)TAM_PADDING; k <= (int)TAM_PADDING; k++) {
                for (int l = -(int)TAM_PADDING; l <= (int)TAM_PADDING; l++) {
                    Pixel vizinho = imagem_lida[i + k][j + l];
                    
                    soma.r += vizinho.r;
                    soma.g += vizinho.g;
                    soma.b += vizinho.b;
                }
            }

            // Média aritmética baseada na área total do kernel
            unsigned int divisor = TAM_KERNEL * TAM_KERNEL;
            Pixel media;
            media.r = (unsigned char)(soma.r / divisor);
            media.g = (unsigned char)(soma.g / divisor);
            media.b = (unsigned char)(soma.b / divisor);

            /* * Mapeamento de Coordenadas:
             * Como a imagem_final nao possui as bordas de padding, subtraímos
             * TAM_PADDING dos índices 'i' e 'j' para normalizar a gravação a partir
             * da coordenada [0][0] do buffer de escrita.
             */
            imagem_final[i - TAM_PADDING][j - TAM_PADDING] = media;
        }
    }

    // Exportando a nova imagem 
    // Gravando metadados
    fprintf(arq_saida, "P6\n");
    fprintf(arq_saida, "%zu %zu\n", largura_original, altura_original);
    fprintf(arq_saida, "%d\n", max_cor);

    // Gravando os resultados
    for (size_t i = 0; i < altura_original; i++) {
        fwrite(imagem_final[i], sizeof(Pixel), largura_original, arq_saida);
    }

    // Fechamento seguro dos descritores de arquivo do sistema
    fclose(arq_entrada);
    fclose(arq_saida);

    // Liberando a memória alocada para as imagens
    desalocar_imagem(imagem_lida, altura_total);
    desalocar_imagem(imagem_final, altura_original);

    // O retorno absoluto de sucesso: o stdout envia o caminho formatado
    printf("%s\n", nome_arq_saida);

    return 0;
}
