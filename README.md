# Matrix Blur Filter

Este projeto apresenta uma aplicação prática de matrizes e álgebra linear no campo da computação gráfica, através da implementação de um filtro de desfoque (*blur*) em imagens digitais no formato **PPM (P6)**. O desenvolvimento foi concebido utilizando uma lógica de processamento de baixo nível inspirada na arquitetura do framework multimedia **FFmpeg**.

## Objetivo

Demonstrar como conceitos abstratos de matrizes e operadores locais de convolução são aplicados diretamente na manipulação de matrizes de píxeis na memória, otimizando o processamento com técnicas avançadas de tratamento de bordas (*edge replication*).

## Lógica e Estrutura de Dados

### Representação de Dados
As cores dos canais RGB da imagem são mapeadas de forma contígua através da estrutura `Pixel`:
```c
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

```

### Alocação de Memória com Padding (Edge Replication)

Para evitar falhas de segmentação ou condições especiais excessivas nas extremidades ao aplicar o kernel de convolução, o programa aloca uma matriz expandida adicionando uma margem (*padding*) de **1 píxel** em cada extremidade (topo, base, esquerda e direita).

### Processamento e Buffering

O algoritmo adota um esquema de duplo espaço de memória para blindar o cálculo contra o efeito de *"borrão em cascata"* (onde um píxel alterado corrompe as leituras dos vizinhos subsequentes na mesma varredura):

1. **Buffer de Leitura:** Matriz expandida com o padding populado e estático, servindo exclusivamente como fonte de dados imutável para as operações matemáticas.
2. **Buffer de Escrita:** Estrutura destino isolada que recebe os novos valores computados, garantindo a integridade dos dados de origem em todo o ciclo do filtro.

### Algoritmo de Convolução

Um kernel de suavização de dimensão **3x3** desliza linearmente sobre a imagem. Para cada coordenada do buffer, o programa realiza o cálculo da média aritmética simples unindo os canais cromáticos do píxel central aos seus 8 vizinhos em volta:

$$\text{NovoPixel}_{R,G,B} = \frac{1}{9} \sum_{i=-1}^{1} \sum_{j=-1}^{1} \text{Vizinho}(x+i, y+j)_{R,G,B}$$

O resultado final é imediatamente projetado na célula correspondente do buffer de escrita.

## Pré-requisitos e Compilação

Para compilar e executar este projeto, necessita de um compilador C padrão (como o `gcc`) instalado no ambiente.

```bash
# Clonar o repositório
git clone https://github.com/CarvDev/matrix-blur-filter.git
cd matrix-blur-filter

# Compilar o programa com otimizações padrão
gcc -O2 main.c -o matrix-blur

```

## Como Utilizar

O programa aceita imagens no formato **PPM P6**.

```bash
./matrix-blur imagem.ppm

```

**Dica:** É possível converter uma imagem para ppm com o comando abaixo (requer ImageMagick)

```bash
magick input.jpg -colorspace sRGB -depth 8 -compress none ppm:output.ppm
```

Esse comando utiliza flags específicas para forçar o formato esperado pelo programa e evitar erros.

## Resultado

Em caso de sucesso, o programa criará o arquivo `imagem-blur.ppm`, contendo a imagem com blur aplicado.

## Feito por:

* [CarvDev](https://github.com/CarvDev)
* [4mandaCardoso](https://github.com/4mandaCardoso)
