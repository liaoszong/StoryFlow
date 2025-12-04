import time
import pathlib
import uuid
from datetime import datetime, timezone, timedelta
import os
import torch
import uvicorn
from diffusers import StableDiffusionPipeline
from fastapi import FastAPI, Form, HTTPException
from pydantic import BaseModel, Field

model_dir = r"D:\StoryFlow\src\local_ai_model_services\models\sd_turbo"
save_dir = r"D:\StoryFlow\src\local_ai_model_services\image"

class ImageReq(BaseModel):
    prompt: str 
    style: str 


class ImageResp(BaseModel):
    image_url: str

pipe = None
def get_pipe():
    global pipe
    if pipe is None:
        pipe = StableDiffusionPipeline.from_pretrained(
            model_dir,
            torch_dtype=torch.float16,
            local_files_only=True,
            safety_checker=None,
        ).to("cuda")
    return pipe

def generate_image(prompt: str, style: str) -> pathlib.Path:
    """返回本地文件路径（相对）"""
    pipe = get_pipe()
    t0 = time.perf_counter()

    # SD-Turbo 推荐 1~4 步 & guidance 0~1.2
    image = pipe(
        prompt,
        num_inference_steps=2,   # SD-Turbo 推荐 1~4 步
        guidance_scale=1.2
    ).images[0]
    
    # 文件名：时间戳 + uuid
    filename = f"img_{int(time.time())}_{uuid.uuid4().hex[:8]}.png"
    file_path = os.path.join(save_dir,filename)
    image.save(file_path)
    cost = (time.perf_counter() - t0) * 1000
    print(f"[图子服务] 生成完成：{file_path}  耗时：{cost:.2f} ms")
    return file_path


app = FastAPI(title="SD-Turbo-子服务")


@app.post("/generate", response_model=ImageResp)
async def generate_image_endpoint(
    prompt: str = Form(...),
    style: str = Form(...)
):
    """接收 Form，返回图片本地 URL"""
    print(f"[图子服务] 收到 prompt={prompt}, style={style}")
    file_path = generate_image(prompt, style)

    # 本地静态 URL（总网关可拼接）
    return ImageResp(image_url=file_path)


# ---------- 启动 ----------
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8002, log_level="info")