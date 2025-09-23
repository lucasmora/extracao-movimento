# Extração de Movimento com Python

![Python](https://img.shields.io/badge/Python-3.11-blue?logo=python&logoColor=white) ![License](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey) ![Status](https://img.shields.io/badge/Status-Active-brightgreen)

Ferramenta desenvolvida em Python para **extrair e evidenciar movimento em vídeos**, analisando diferenças entre frames consecutivos. O código gera vídeos de saída com alterações de movimento claramente destacadas, permitindo a visualização detalhada de padrões que seriam difíceis de perceber a olho nu.  
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


## Exemplo visual

![normal_gif00086400](https://github.com/user-attachments/assets/af0543cc-5769-4913-afec-978bd35dfb5f)  
_Entrada_

![cinza_gif00086400](https://github.com/user-attachments/assets/81b526a6-3df1-4271-9a42-65d25b3eef3d)  
_Saída_ 

---


## Licença

Este projeto está licenciado sob **CC BY-NC 4.0** — uso **não comercial** com atribuição.  
[Saiba mais](https://creativecommons.org/licenses/by-nc/4.0/)
