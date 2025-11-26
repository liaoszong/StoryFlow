from diffusers import StableDiffusionPipeline
import torch

model_dir = r"D:\StoryFlow\src\local_ai_model_services\models\sd_turbo"

pipe = StableDiffusionPipeline.from_pretrained(
    model_dir,
    torch_dtype=torch.float16,
    local_files_only=True,
    safety_checker=None
).to("cuda")

prompt = "A cute cat sleeping on a university campus, anime style"

image = pipe(
    prompt,
    num_inference_steps=2,   # SD-Turbo 推荐 1~4 步
    guidance_scale=1.2
).images[0]

save_path = r"D:\StoryFlow\src\local_ai_model_services\image\scene_turbo_1.png"
image.save(save_path)

print("图片已生成:", save_path)
