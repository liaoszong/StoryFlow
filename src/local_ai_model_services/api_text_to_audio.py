import os
import uuid
import time
from pathlib import Path
from datetime import datetime, timezone, timedelta
import torchaudio
from fastapi import FastAPI, Form, HTTPException
from pydantic import BaseModel, Field
import uvicorn

output_dir = Path("D:/StoryFlow/src/local_ai_model_services/audio")
model_dir = Path("D:/StoryFlow/src/local_ai_model_services/models/CosyVoice")
os.makedirs(output_dir, exist_ok=True)
import sys
sys.path.append(str(model_dir))
sys.path.append(
    "D:/StoryFlow/src/local_ai_model_services/models/CosyVoice/third_party/Matcha-TTS"
)
from cosyvoice.cli.cosyvoice import CosyVoice, CosyVoice2
from cosyvoice.utils.file_utils import load_wav

# 零样本提示音（固定）
PROMPT_WAV = model_dir / "asset/zero_shot_prompt.wav"
cosyvoice = CosyVoice2(
    'D:/StoryFlow/src/local_ai_model_services/models/CosyVoice/pretrained_models/CosyVoice2-0.5B',
    load_jit=False, load_trt=False, load_vllm=False, fp16=False
)
prompt_speech_16k = load_wav(str(PROMPT_WAV), 16000)

class TtsReq(BaseModel):
    narration: str 

class TtsResp(BaseModel):
    audio_url: str

def generate_tts(text: str) -> Path:
    """返回本地 WAV 文件路径（相对）"""
    t0 = time.perf_counter()  
    filename = f"tts_{int(time.time())}_{uuid.uuid4().hex[:8]}.wav"
    file_path = output_dir / filename
    for i, j in enumerate(cosyvoice.inference_zero_shot(text, '希望你以后能够做的比我还好呦。', prompt_speech_16k, stream=False)):
        torchaudio.save(str(file_path), j['tts_speech'], cosyvoice.sample_rate)
    cost = (time.perf_counter() - t0) * 1000
    print(f"[TTS子服务] 生成完成：{file_path}  耗时：{cost:.2f} ms")
    return file_path

app = FastAPI(title="CosyVoice-TTS-子服务")

@app.post("/generate", response_model=TtsResp)
async def ggenerate_tts_endpoint(
    narration: str = Form(...),
):
    print(f"[TTS子服务] 收到 text={narration[:30]}")
    file_path = generate_tts(narration)

    # 返回本地静态 URL
    url = f"/files/audio/{file_path.name}"
    return TtsResp(audio_url=url)

# ---------- 启动 ----------
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8003, log_level="info")