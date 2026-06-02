#!/bin/bash
# Cron Job para limpeza de imagens residuais 

# Captura dinamicamente a pasta onde o próprio script cronjob.sh está guardado
PASTA_DO_SCRIPT=$(cd "$(dirname "$0")" && pwd)

# Define o alvo apontando corretamente para o subdiretório de imagens
DIRETORIO="$PASTA_DO_SCRIPT/public/imagens"

# Encontra e apaga ficheiros (.jpg e .ppm) modificados há mais de 10 minutos
find "$DIRETORIO" -type f \( -name "*.jpg" -o -name "*.ppm" \) -mmin +10 -delete