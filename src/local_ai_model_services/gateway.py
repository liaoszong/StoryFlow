import os
import time
import httpx
from fastapi import FastAPI, HTTPException, Form
from pydantic import BaseModel, Field
import uvicorn
from typing import Dict

# ---------- 子服务表 ----------
SERVICES = {
    "llm": "http://localhost:8001/generate",
    "tti": "http://localhost:8002/generate",
    "tta": "http://localhost:8003/generate",
    "itv": "http://localhost:8004/generate",
}


app = FastAPI(title="AI-多模型网关")
@app.post("/{model_id}/generate")
async def single_generate(
    model_id: str,
    raw_text: str = Form(default=""),
    style: str = Form(default=""),
    prompt: str = Form(default=""),
    narration:str = Form(default=""),
    image_url: str = Form(default="")
):
    if model_id not in SERVICES:
        raise HTTPException(404, "模型不存在")
    url = SERVICES[model_id]

    # 构造子服务参数，子服务自己决定用哪些字段
    if model_id == "llm":
        payload = {
            "raw_text": raw_text,
            "style": style,
        }
    elif model_id == "tti":
        payload = {
            "prompt": prompt,
            "style": style
        }
    elif model_id == "tta":
        payload = {
            "narration": narration,
        }
    elif model_id == "itv":
        payload = {
            "image_url": image_url
        }

    async with httpx.AsyncClient(timeout=120) as cli:
        resp = await cli.post(url, data=payload)
        resp.raise_for_status()
    return resp.json()  # 原样返回子服务 JSON

if __name__ == "__main__":
    uvicorn.run("gateway:app", host="0.0.0.0", port=9000, log_level="info")