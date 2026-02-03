# HotfixServer 用户使用文档

## 启动服务
推荐使用统一脚本：
- Windows：`run.bat start`
- Debian/Ubuntu：`./run.sh start`

## 参数说明
- `--upload-dir`：上传目录（默认 `uploads`）
- `--static-dir`：前端目录（默认 `static`）
- `--max-files`：每个模式最大保留文件数（默认 20）
- `--config`：配置文件路径（包含删除白名单）
- `--trust-x-forwarded-for`：信任代理头（仅在内网代理下使用）
- `--log-level`：日志等级（可被 `config.json` 覆盖）
- `--log-file`：日志输出文件（默认 `run/hotfix_server.log`）

## 上传文件
仅支持通过 HTTP 接口上传（前端不提供上传入口）：
```bash
curl -F "file=@./h75_hotfix_demo.bin" http://127.0.0.1:8000/upload
```
文件名需满足模式关键字，否则会被拒绝。`config.json` 支持按模式设置 `mode_limits`。

## 配置文件示例
```json
{
  "delete_allow": ["127.0.0.1", "::1"],
  "max_files_per_mode": 20,
  "log_level": "INFO",
  "mode_limits": {
    "h75_hotfix": { "max_files": 20 }
  }
}
```

## 浏览与下载
浏览器访问：
```
http://127.0.0.1:8000/
```
点击卡片中的 Download 按钮即可下载。

## 删除文件
仅白名单 IP 可见删除按钮；也可用命令行：
```bash
curl -X DELETE http://127.0.0.1:8000/files/h75_hotfix_demo.bin
```

## 关闭服务
```powershell
scripts\stop_server.ps1
```
或：
```bat
scripts\stop_server.cmd
```

## 运维脚本
- 重启：`scripts\restart_server.ps1`
- 状态：`scripts\status_server.ps1`
- 查看日志：`scripts\logs_server.ps1 -Lines 200`

## 统一运行脚本
- `run.bat start|stop|restart|status|logs`
- `./run.sh start|stop|restart|status|logs`
