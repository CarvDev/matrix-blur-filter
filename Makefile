# Compilador e flags de compilação
CC = x86_64-redhat-linux-gcc-8
CFLAGS = -Wall -Wextra -O2 -std=c99

# Caminhos dos ficheiros
SRC = src/blur.c
TARGET = src/blur_program

# Arquivo de script server-side
SERVER_SCRIPT = server.js

# Alvo principal: compila o código fonte
all: build

# Regra para gerar o executável final em C
# A compilação utiliza a flag -O3 para garantir a máxima otimização de desempenho 
build: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Inicia o servidor backend Node.js
# Depende do alvo 'build' para garantir que o binário em C está compilado 
run: build $(SERVER_SCRIPT)
	node $(SERVER_SCRIPT)

# Remove o binário compilado e as imagens residuais geradas durante a execução
clean:
	rm -f $(TARGET)
	rm -f public/imagens/*blur.ppm
