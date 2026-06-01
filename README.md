# Matrix Blur Filter

Este projeto implementa um filtro de desfoque (*blur*) em imagens digitais usando uma combinação de processamento em C e uma API Node.js. O objetivo é demonstrar como uma convolução 3x3 pode ser aplicada diretamente sobre a matriz de pixels de uma imagem.

## Objetivo

Mostrar como um filtro local de média sobre um kernel 3x3 suaviza uma imagem mantendo a integridade dos dados por meio de buffers separados e tratamento de bordas (*padding* / *edge replication*).

## Como Funciona

- O código C (`src/blur.c`) lê uma imagem no formato PPM, aplica um filtro 3x3 e gera uma imagem PPM borrada.
- O servidor Node.js (`server.js`) recebe uploads, converte a imagem para PPM, executa o binário C e converte o resultado para JPG.
- A interface web está em `public/index.html`.

## Requisitos
- Docker 

OU:

- Node.js
- npm
- gcc
- make
- GraphicsMagick

## Opção 1: Execução via Docker (recomendado)

1. Clone o repositório:
   ```bash
git clone 'https://github.com/CarvDev/matrix-blur-filter.git'
cd matrix-blur-filter
```

2. Construa a imagem Docker:
   ```bash
docker build -t matrix-blur-app .
```

3. Inicie o container:
   ```bash
docker run -d -p 3000:3000 --name blur-app matrix-blur-app
```

4. Abra no navegador:
   ```text
http://localhost:3000
```

> Para parar e remover o container:
> ```bash
docker stop blur-app && docker rm blur-app
```

## Opção 2: Execução local

1. Clone o repositório:
   ```bash
git clone 'https://github.com/CarvDev/matrix-blur-filter.git'
cd matrix-blur-filter
```

2. Instale as dependências Node.js:
   ```bash
npm install
```

3. Compile e execute o servidor:
   ```bash
make run
```

4. Abra no navegador:
   ```text
http://localhost:3000
```

## Comandos úteis

- `make build` - compila `src/blur.c` em `src/blur_program`
- `make run` - compila o binário e inicia o servidor Node.js
- `make clean` - remove o executável compilado e artefatos temporários

## Autor(es)

- [CarvDev](https://github.com/CarvDev)
- [4mandaCardoso](https://github.com/4mandaCardoso)
