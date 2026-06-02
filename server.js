const express = require('express');
const multer = require('multer');
const { exec } = require('child_process');
const util = require('util');
const fs = require('fs');
const path = require('path');

// Promisify permite a utilização de async/await em chamadas do sistema operativo
const execPromise = util.promisify(exec);

const app = express();

/*
 * Configuração do Multer
 * O middleware guarda ficheiros de qualquer formato suportado pelo 
 * ImageMagick/GraphicsMagick. A conversão é feita a jusante na pipeline.
 */
const crypto = require('crypto');
const storage = multer.diskStorage({
    destination: (req, file, cb) => {
        cb(null, 'public/imagens/'); 
    },
    filename: (req, file, cb) => {
        const extensao = path.extname(file.originalname) || '.jpg';
        // Gera um nome como: 550e8400-e29b-41d4-a716-446655440000.jpg
        const nomeSeguro = crypto.randomUUID(); 
        cb(null, `${nomeSeguro}${extensao}`); 
    }
});
const upload = multer({ storage });

app.use(express.static('public'));

app.post('/aplicar-blur', upload.single('image'), async (req, res) => {
    if (!req.file) {
        return res.status(400).json({ erro: "Nenhum ficheiro recebido." });
    }

    // Captura o valor do kernel enviado pelo front-end.
    // Usa parseInt para segurança (evita injeção de comandos no shell) e um fallback para 3.
    const kernelTamanho = parseInt(req.body.kernel) || 3;

    const imgOriginal = req.file.path;
    const baseName = path.basename(imgOriginal, path.extname(imgOriginal));
    const dirName = path.dirname(imgOriginal);

    /*
     * Definição da pipeline de ficheiros
     * O binário C injeta nativamente o sufixo '-blur' no output.
     * É imperativo garantir que o caminho imgPpmBlur reflete a saída hardcoded no C.
     */
    const imgPpm = path.join(dirName, `${baseName}.ppm`);
    const imgPpmBlur = path.join(dirName, `${baseName}-blur.ppm`);
    const imgFinalJpg = path.join(dirName, `${baseName}-resultado.jpg`);

    const executavelC = './src/blur_program';

    try {
        /*
         * 1. Pré-processamento
         * GraphicsMagick vai gerar o formato P6 padrão ao exportar para "ppm:".
         */
        const cmdToPpm = `gm convert "${imgOriginal}" -background white -flatten -type TrueColor -depth 8 "ppm:${imgPpm}"`;
        await execPromise(cmdToPpm);

        /*
         * 2. Processamento do Núcleo (Kernel dinâmico)
         * O parâmetro -k é injetado antes do nome do arquivo.
         * O C lê imgPpm e gera imgPpmBlur.
         */
        const cmdRunC = `${executavelC} -k ${kernelTamanho} "${imgPpm}"`;
        await execPromise(cmdRunC);

        /*
         * 3. Pós-processamento
         * Converter o PPM finalizado para JPG por questões de interoperabilidade web.
         */
        const cmdToJpg = `gm convert "${imgPpmBlur}" "${imgFinalJpg}"`;
        await execPromise(cmdToJpg);

        /*
         * 4. Resposta e Limpeza
         * Os artefactos intermédios (.ppm) e a origem devem ser removidos 
         * para evitar sobrecarga de inodes. O JPG é retido para servir o pedido do Frontend.
         */
        const caminhoWeb = imgFinalJpg.replace(/^public[\/\\]/, '/');
        res.json({ arquivo: caminhoWeb });

        // Limpeza assíncrona em background
        fs.unlink(imgOriginal, () => {});
        fs.unlink(imgPpm, () => {});
        fs.unlink(imgPpmBlur, () => {});

        // Agenda a deleção do JPG final para 5 minutos (300.000 ms)
        setTimeout(() => {
            fs.unlink(imgFinalJpg, (err) => {
                if (!err) console.log(`Arquivo limpo (Timeout): ${imgFinalJpg}`);
            });
        }, 5 * 60 * 1000);

    } catch (error) {
        // Fallback de limpeza caso a pipeline falhe a meio
        if (fs.existsSync(imgOriginal)) fs.unlinkSync(imgOriginal);
        if (fs.existsSync(imgPpm)) fs.unlinkSync(imgPpm);
        if (fs.existsSync(imgPpmBlur)) fs.unlinkSync(imgPpmBlur);

        const msgErro = error.stderr ? error.stderr.trim() : error.message;
        return res.status(500).json({ erro: `Falha na Pipeline: ${msgErro}` });
    }
});

app.listen(3000, () => {
    console.log('Servidor Pipeline ligado na porta 3000');
});

setInterval(() => {
    const diretorio = path.join(__dirname, 'public', 'imagens');
    fs.readdir(diretorio, (err, files) => {
        if (err) return;
        const agora = Date.now();
        
        files.forEach(file => {
            // Impede que o gari delete o arquivo de estrutura do Git
            if (file === '.gitkeep') return;

            const caminho = path.join(diretorio, file);
            fs.stat(caminho, (err, stats) => {
                if (err) return;
                // Se o arquivo tiver mais de 10 minutos de vida, apaga
                if (agora - stats.birthtimeMs > 10 * 60 * 1000) {
                    fs.unlink(caminho, () => {});
                }
            });
        });
    });
}, 60 * 60 * 1000); // Executa a varredura a cada 1 hora