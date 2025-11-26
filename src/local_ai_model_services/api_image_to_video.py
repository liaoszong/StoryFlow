import time
import uuid
from pathlib import Path
from datetime import datetime, timezone, timedelta
import torch
import imageio
from PIL import Image
from fastapi import FastAPI, Form, HTTPException, UploadFile, File
from pydantic import BaseModel, Field
import uvicorn
from diffusers import StableVideoDiffusionPipeline

model_dir = Path("D:/StoryFlow/src/local_ai_model_services/models/stable-video-diffusion-img2vid-xt-1-1")
save_dir = Path("D:/StoryFlow/src/local_ai_model_services/video")
save_dir.mkdir(parents=True, exist_ok=True)

pipe = None
def get_pipe():
    global pipe
    if pipe is None:
        pipe = StableVideoDiffusionPipeline.from_pretrained(
            model_dir,
            torch_dtype=torch.float16,
            local_files_only=True,
        )
    try:
        pipe.enable_model_cpu_offload()     # 在显存不足时很有用
    except:
        pass
    return pipe

class VideoReq(BaseModel):
    image_url: str

class VideoResp(BaseModel):
    video_url: str

def generate_video(init_img_path: str) -> Path:
    t0 = time.perf_counter()

    # 1. 加载初始图
    init_image = Image.open(init_img_path)

    # 2. 推理
    pipe = get_pipe()
    out = pipe(
        image=init_image,
        width=384,
        height=384,
        num_frames=30,
        fps=6,
        motion_bucket_id=30,
        num_inference_steps=25,
        decode_chunk_size=1,
    )

    # 3. 帧 → imageio
    video_frames = out.frames[0]  # List[PIL.Image]
    safe_frames = []
    for img in video_frames:
        img = img.convert("RGB")
        safe_frames.append(img)

    # 4. 保存 mp4
    filename = f"vid_{int(time.time())}_{uuid.uuid4().hex[:8]}.mp4"
    file_path = save_dir / filename
    imageio.mimwrite(
        file_path,
        safe_frames,
        fps=6,
        quality=8,
    )

    cost = (time.perf_counter() - t0) * 1000
    print(f"[视频子服务] 生成完成：{file_path}  耗时：{cost:.2f} ms")
    return file_path

app = FastAPI(title="StableVideo-子服务")
@app.post("/generate", response_model=VideoResp)
async def generate_video_endpoint(
    image_url: str = Form(...)
):
    print(f"[视频子服务] 收到 image_url={image_url[:30]}.")

    # 1. 零样本：用固定初始图（未来可上传）
    init_img_path = Path("D:/StoryFlow/src/local_ai_model_services/image/scene_turbo.png")

    # 2. 生成
    file_path = generate_video(image_url)

    # 3. 返回本地 URL
    url = str(file_path)
    return VideoResp(video_url=url)

# ---------- 启动 ----------
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8004, log_level="info")