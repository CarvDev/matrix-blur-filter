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

## Como Executar

Você pode executar a aplicação de duas formas: utilizando o Docker (recomendado para garantir o isolamento do ambiente) ou globalmente, instalando as dependências diretamente no sistema operacional.

### Opção 1: Execução via Docker (Recomendado)

Certifique-se de ter o Docker instalado, o serviço em execução na sua máquina e a porta 3000 aberta. Na raiz do projeto, execute os comandos abaixo:

1. **Clonar o repositório:**
   ```bash
git clone 'https://github.com/CarvDev/matrix-blur-filter.git'
cd matrix-blur-filter

```

2. **Construir a imagem da aplicação:**
   ```bash
docker build -t matrix-blur-app .

```

3. **Iniciar o container em background:**
```bash
docker run -d -p 3000:3000 --name blur-app matrix-blur-app

```

4. **Acessar a interface:**
Abra o seu navegador e acesse: `http://localhost:3000`

> **Nota:** Para parar e remover o servidor posteriormente, utilize `docker stop blur-app && docker rm blur-app`.

---

### Opção 2: Execução Global (Nativa)

As instruções abaixo requerem os programas `nodejs`, `npm`, `gcc` `make` e `graphicsmagick`.

1. **Clonar o repositório:**
   ```bash
git clone 'https://github.com/CarvDev/matrix-blur-filter.git'
cd matrix-blur-filter

```

2. **Preparar o ambiente do projeto:**
Na raiz do repositório, utilize o npm para instalar as dependências do Node.js (Express e Multer):
```bash
npm install 

```

3. **Compilar o binário C e iniciar o servidor:**
O comando abaixo compila o código-fonte em C com otimização máxima (`-O3`) e levanta a API Node.js em seguida:
```bash
make run

```

4. **Acessar a interface:**
Abra o seu navegador e acesse: `http://localhost:3000`

## Feito por:

* [CarvDev](https://github.com/CarvDev)
* [4mandaCardoso](https://github.com/4mandaCardoso)
