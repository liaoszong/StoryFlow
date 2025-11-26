from diffusers import StableVideoDiffusionPipeline
import torch
from PIL import Image
import numpy as np
import imageio

# -----------------------------
# 1. 加载管道
# -----------------------------
pipe = StableVideoDiffusionPipeline.from_pretrained(
    "./models/stable-video-diffusion-img2vid-xt-1-1",
     torch_dtype=torch.float16,       # 半精度
)

# 显存优化
# pipe.enable_sequential_cpu_offload()
# pipe.enable_attention_slicing()
# try:
#     pipe.enable_vae_slicing()           # 节省 VAE 推理显存
# except:
#     pass

try:
    pipe.enable_model_cpu_offload()     # 在显存不足时很有用
except:
    pass

# -----------------------------
# 2. 读取初始图像
# -----------------------------
init_image = Image.open("./image/scene_turbo.png")
# init_prompt = "A cat is walking."
# -----------------------------
# 3. 推理生成视频帧
# -----------------------------
out = pipe(
    image=init_image,
    height=384,
    width=384,
    num_frames=30,          # 总帧数
    motion_bucket_id=30,   # 推荐值
    num_inference_steps=25, # 步数，可调
    decode_chunk_size=1,         # 关键：按帧解码
    fps=6
)

video_frames = out.frames[0]  # frames 是 [[PIL1, PIL2,...]] 形式

# -----------------------------
# 4. 保存视频
# -----------------------------
safe_frames = []
for img in video_frames:
    img = img.convert("RGB")
    safe_frames.append(img)

video_path = "./video/output_video.mp4"
imageio.mimwrite(
    video_path,
    safe_frames,
    fps=6,
    quality=8
)
print("视频已保存到", video_path)
