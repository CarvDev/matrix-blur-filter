#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Analisa o nome do arquivo de entrada e anexa a string "-blur.ppm"
 * antes da extensão para gerar o arquivo de saída esperado pelo backend.
 */
void generate_output_filename(const char *input_name, char *output_name) {
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

int main(int argc, char *argv[]) {
    // Validação estrita da linha de comandos esperada pelo Node.js
    if (argc != 2) {
        fprintf(stderr, "Erro de sintaxe. Uso esperado: %s <caminho_imagem.ppm>\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    char output_filename[256];
    
    generate_output_filename(input_filename, output_filename);

    // Abertura explícita em modo binário ("rb" e "wb") para preservar 
    // a integridade de ficheiros de imagem (crucial para PPM formato P6)
    FILE *in_file = fopen(input_filename, "rb");
    if (!in_file) {
        fprintf(stderr, "Erro ao abrir o arquivo de origem: %s\n", input_filename);
        return 1;
    }

    FILE *out_file = fopen(output_filename, "wb");
    if (!out_file) {
        fprintf(stderr, "Erro ao criar o arquivo de destino: %s\n", output_filename);
        fclose(in_file);
        return 1;
    }

    // Buffer de 4KB para minimizar operações de I/O no disco durante a cópia mock
    unsigned char buffer[4096];
    size_t bytes_read;

    // Cópia 1:1 do arquivo para simular o tempo e o resultado do processamento
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_file)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, out_file);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Erro fatal de I/O ao gravar os dados na imagem gerada.\n");
            fclose(in_file);
            fclose(out_file);
            return 1;
        }
    }

    // Fechamento seguro dos descritores de arquivo do sistema
    fclose(in_file);
    fclose(out_file);

    // O retorno absoluto de sucesso: o stdout envia o caminho formatado
    printf("%s\n", output_filename);

    return 0;
}
