# start_all.ps1  (PowerShell 版)
# 用法：右键「使用 PowerShell 运行」或终端：powershell -File start_all.ps1

# 1. 日志目录
$logDir = "D:\StoryFlow\src\local_ai_model_services\logs"
if (!(Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir }

Write-Host "==== Launch 5 sub-services and 1 master-services(PowerShell)====" -ForegroundColor Green

# 2. 并发启动 5 个虚拟环境 + Python 脚本
Start-Process -FilePath "ollama" -ArgumentList "serve" -WindowStyle Hidden -RedirectStandardOutput "$PSScriptRoot\logs\ollama.log" -RedirectStandardError "$PSScriptRoot\logs\ollama.err"

Start-Job -Name "LLM-8001" -ScriptBlock {
    Set-Location D:\StoryFlow\src\local_ai_model_services
    python .\api_llm.py > D:\StoryFlow\src\local_ai_model_services\logs\llm.log 2>&1
}

Start-Job -Name "TTI-8002" -ScriptBlock {
    conda activate sd_turbo
    Set-Location D:\StoryFlow\src\local_ai_model_services
    python .\api_text_to_image.py > D:\StoryFlow\src\local_ai_model_services\logs\tti.log 2>&1
}
Start-Process -FilePath "conda" -ArgumentList "run -n cosyvoice python D:\StoryFlow\src\local_ai_model_services\api_text_to_audio.py" -WindowStyle Hidden -RedirectStandardOutput "D:\StoryFlow\src\local_ai_model_services\logs\tta.log" -RedirectStandardError "D:\StoryFlow\src\local_ai_model_services\logs\tta.err"
Start-Job -Name "TTA-8003" -ScriptBlock {
    conda activate cosyvoice
    Set-Location D:\StoryFlow\src\local_ai_model_services
    python .\api_text_to_audio.py > D:\StoryFlow\src\local_ai_model_services\logs\tta.log 2>&1
}
# 8004 子服务：StableVideo Diffusion 后台启动
Start-Process -FilePath "D:\software\miniconda3\envs\svd_imag2vid\python.exe" -ArgumentList "D:\StoryFlow\src\local_ai_model_services\api_image_to_video.py" -WindowStyle Hidden -RedirectStandardOutput "D:\StoryFlow\src\local_ai_model_services\logs\itv.log" -RedirectStandardError "D:\StoryFlow\src\local_ai_model_services\logs\itv.err"
Start-Job -Name "ITV-8004" -ScriptBlock {
    conda activate svd_imag2vid
    Set-Location D:\StoryFlow\src\local_ai_model_services
    python .\api_image_to_video.py > D:\StoryFlow\src\local_ai_model_services\logs\itv.log 2>&1
}

Start-Job -Name "GATEWAY-9000" -ScriptBlock {
    conda activate base
    Set-Location D:\StoryFlow\src\local_ai_model_services
    python .\gateway.py > D:\StoryFlow\src\local_ai_model_services\logs\gateway.log 2>&1
}

Write-Host "==== start all jobs, logs in $logDir ====" -ForegroundColor Green
Write-Host "press any key to close all jobs"
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

# 3. 一键关闭所有作业
Get-Job | Stop-Job -Force
Get-Job | Remove-Job -Force
Write-Host "close all jobs" -ForegroundColor Red