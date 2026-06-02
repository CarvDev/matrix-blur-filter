#!/bin/bash
# Cron Job para limpeza de imagens residuais 

# Define o diretório exato (alterar para o caminho absoluto do servidor)
DIRETORIO=""

# Encontra e apaga ficheiros (.jpg e .ppm) modificados há mais de 10 minutos
find "$DIRETORIO" -type f \( -name "*.jpg" -o -name "*.ppm" \) -mmin +10 -delete