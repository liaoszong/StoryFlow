import json
import pathlib
from typing import List, Optional
from pydantic import BaseModel, Field
import httpx
import time, uuid
from datetime import datetime, timezone, timedelta
from fastapi import FastAPI,Form
import uvicorn
import os

save_dir = r"D:\StoryFlow\src\local_ai_model_services\json"
class SceneItem(BaseModel):
    scene_title: str
    prompt: str
    narration: str
    transition: str


class StoryBoard(BaseModel):
    id:str
    create_time:str
    raw_story: str
    style: str
    bgm_suggestion: str
    scenes: List[SceneItem]

def call_ollama(raw_story: str, style: str) -> dict:
    """
    调用 Ollama 接口将故事转换为标准分镜JSON并返回字典。
    """
    prompt = f"""
    请根据以下故事内容与风格，生成几个形容词形容故事氛围，填入 bgm_suggestion 字段，
    并生成三个分镜（scene_1、scene_2、scene_3），
    每个分镜需包含：scene_title、prompt、narration、transition 四个字段, scene_title 为分镜标题，prompt 为分镜描述，narration 为分镜旁白，transition为视频过渡方式，一般为淡入/切换/淡出。
    
    故事内容："{raw_story}"
    故事风格："{style}"

    请将生成的内容完整填写到以下 JSON 模板中，所有字段必须生成内容，并确保输出为严格的 JSON 格式：

    {{
    "id": str(uuid.uuid4()),
    "raw_story": "{raw_story}",
    "style": "{style}",
    "bgm_suggestion": "",
    "scenes": [
        {{
        "scene_title": "",
        "prompt": "",
        "narration": "",
        "transition": ""
        }},
        {{
        "scene_title": "",
        "prompt": "",
        "narration": "",
        "transition": ""
        }},
        {{
        "scene_title": "",
        "prompt": "",
        "narration": "",
        "transition": ""
        }}
    ]
    }}
    """
    payload = {
        "model": "qwen2.5-coder:7b",
        "prompt": prompt,
        "format": "json",
        "stream": False,
    }

    resp = httpx.post(
        "http://localhost:11434/api/generate",
        json=payload,
        timeout=60
    )
    resp.raise_for_status()

    # Ollama 返回格式为 {"response": "...json 字符串..."}
    return json.loads(resp.json()["response"])

def fix_missing_fields(raw_dict: dict) -> dict:
    """自动补齐缺失字段，并为 3 个场景填入固定 transition 值"""
    scenes = raw_dict.get("scenes", [])
    fixed_scenes = []
    DEFAULT_TRANSITIONS = ["淡入", "切换", "淡出"]
    for i, scene in enumerate(scenes):
        # 自动填充 transition
        if i < len(DEFAULT_TRANSITIONS):
            scene["transition"] = DEFAULT_TRANSITIONS[i]
        else:
            # 多于 3 个场景则默认用“切换”
            scene["transition"] = "切换"

        fixed_scenes.append(scene)

    raw_dict["scenes"] = fixed_scenes
    return raw_dict
def ensure_create_time(story_dict: dict) -> dict:
    """
    补全 StoryBoard 数据字典中的 createTime 字段。
    使用中国标准时间（UTC+8）
    Args:
        story_dict (dict): 原始 StoryBoard 字典
    Returns:
        dict: 补全 createTime 后的字典
    """
    if "createTime" not in story_dict or not story_dict["createTime"]:
        cst = timezone(timedelta(hours=8))
        story_dict["create_time"] = datetime.now(cst).isoformat()
    return story_dict

app = FastAPI(title="LLM")
@app.post("/generate")
async def generate_storyboard(
    raw_text: str = Form(...),
    style: str = Form(...)
):
    print(f"[子服务] 收到：{raw_text=}, {style=}")   # ← 日志
    raw_dict = call_ollama(raw_text, style)
    fixed_missing_dict = fix_missing_fields(raw_dict)
    fixed_dict = ensure_create_time(fixed_missing_dict)

    # 校验 & 规范输出
    board = StoryBoard.model_validate(fixed_dict)
     # 文件名：时间戳 + uuid
    filename = f"json_{int(time.time())}_{uuid.uuid4().hex[:8]}.json"
    file_path = os.path.join(save_dir,filename)
    pathlib.Path(file_path).write_text(
        board.model_dump_json(ensure_ascii=False, indent=2),
        encoding="utf-8"
    )
    print("[子服务] 返回成功")                     # ← 日志
    return board.model_dump()

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8001,log_level="info")