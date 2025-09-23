import cv2
from collections import deque
from tqdm import tqdm
import argparse
import os

# Configuração dos argumentos de linha de comando
parser = argparse.ArgumentParser(description="Gerar vídeo de diferença com delay de frames")
parser.add_argument("entrada", type=str, help="Caminho do arquivo de vídeo de entrada")
parser.add_argument("--delay_frames", type=int, default=5,
                    help="Número de frames de atraso para comparação (padrão: 5)")

args = parser.parse_args()

entrada = args.entrada
delay_frames = args.delay_frames
# Gerar nome de saída automaticamente
nome_base, ext = os.path.splitext(os.path.basename(entrada))
saida = f"{nome_base}_{delay_frames}frames.mp4"

def gerar_video_de_diferenca(entrada):
    print(f"Processando vídeo usando {delay_frames} frames de diferença...")
    
    # Ativar OpenCL para GPU AMD
    cv2.ocl.setUseOpenCL(True)
    print("OpenCL disponível:", cv2.ocl.useOpenCL())

    cap = cv2.VideoCapture(entrada)
    fps = cap.get(cv2.CAP_PROP_FPS)
    largura  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    altura   = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fourcc   = cv2.VideoWriter_fourcc(*"mp4v")
    out = cv2.VideoWriter(saida, fourcc, fps, (largura, altura))

    buffer = deque(maxlen=delay_frames)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

    for _ in tqdm(range(total_frames), desc="Processando vídeo"):
        ret, frame = cap.read()
        if not ret:
            break

        # Copia do frame atual para output
        output = frame.copy()

        # Se o buffer está cheio, aplicar efeito no frame atrasado
        if len(buffer) == delay_frames:
            delayed_frame = buffer[0]
            frame_gpu = cv2.UMat(output)
            delayed_gpu = cv2.UMat(delayed_frame)
            inverted_gpu = cv2.bitwise_not(delayed_gpu) # Inverte as cores da imagem
            output_gpu = cv2.addWeighted(frame_gpu, 0.5, inverted_gpu, 0.5, 0) # Define opacidade para 50%
            output = output_gpu.get()

        # Adicionar frame atual no buffer
        buffer.append(frame.copy())

        # Escrever frame processado imediatamente
        out.write(output)

    cap.release()
    out.release()
    print("Vídeo processado e salvo em", saida)

if __name__ == "__main__":
    gerar_video_de_diferenca(entrada)
