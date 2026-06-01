# Usa a imagem oficial do Node.js (versão 20 LTS) baseada em Debian Slim
FROM node:20-bookworm-slim

# Define o diretório de trabalho dentro do container
WORKDIR /usr/src/app

# Atualiza os repositórios e instala o compilador C, o Make e o GraphicsMagick
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    graphicsmagick \
    && rm -rf /var/lib/apt/lists/* # O rm -rf limpa o cache do apt para deixar a imagem final mais leve

# Copia apenas os arquivos de dependência primeiro (otimiza o cache do Docker)
COPY package*.json ./

# Instala as dependências do Node.js (Express, Multer)
RUN npm install

# Copia o restante do código do projeto para o container
COPY . .

# Compila o programa em C utilizando o Makefile
RUN make build

# Garante que a pasta de imagens exista e tenha as permissões corretas
RUN mkdir -p public/imagens && chmod 777 public/imagens

# Expõe a porta que o Express vai utilizar
EXPOSE 3000

# Comando principal para manter o container rodando
CMD ["node", "server.js"]