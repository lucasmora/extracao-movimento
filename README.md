# Motion Extraction com Python

![Python](https://img.shields.io/badge/Python-3.11-blue?logo=python&logoColor=white) ![License](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey) ![Status](https://img.shields.io/badge/Status-Active-brightgreen)

**Motion Extraction** é uma ferramenta desenvolvida em Python para **extrair e evidenciar movimento em vídeos**, analisando diferenças entre frames consecutivos. O projeto gera vídeos de saída com alterações de movimento claramente destacadas, permitindo a visualização detalhada de padrões que seriam difíceis de perceber a olho nu.  
Inspirado pelo vídeo [Motion Extraction](https://www.youtube.com/watch?v=NSS6yAMZF78), o projeto foi adaptado para Python com suporte a múltiplos formatos de vídeo e aceleração via GPU (OpenCL).

---

## Funcionalidades principais

- Processamento quadro a quadro de vídeos para evidenciar movimento de forma precisa.
- Geração de novos vídeos com alterações de movimento destacadas.
- Suporte a diferentes formatos e resoluções de vídeo.
- Aceleração opcional via GPU usando OpenCL.

---

## Dependências do projeto
- opencv-python
- numpy
- tqdm

---

## Uso básico

```bash
python extrator-movimento.py caminho/do/video.mp4
```

### Parâmetros opcionais

- `--delay_frames`: número de frames para atrasar a comparação. Valores maiores permitem evidenciar mudanças acumuladas no vídeo (padrão: 5).


## Licença

Este projeto está licenciado sob **CC BY-NC 4.0** — uso **não comercial** com atribuição.  
[Saiba mais](https://creativecommons.org/licenses/by-nc/4.0/)
