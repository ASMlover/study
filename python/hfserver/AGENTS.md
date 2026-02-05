# Repository Guidelines

## Project Background
This repository implements HotfixServer, an internal HTTP file management system for distributing hotfix packages. It focuses on controlled uploads, mode-based retention policies, and a modern web UI for listing, downloading, and deleting files in a dev environment.

## Project Structure & Module Organization
Key paths in this repository:
- `hotfix_server.py` for the HTTP server implementation.
- `hotfixserver/` for backend modules (config, logging, storage, HTTP server).
- `static/` for the web UI (HTML/CSS/JS).
- `uploads/` for runtime file storage (created at runtime).
- `scripts/` for start/stop/restart/status/log helpers.
- `docs/` for design, user, and test documentation.

## Build, Test, and Development Commands
This project uses the Python standard library (no external dependencies).
- Run the server: `python hotfix_server.py --host 0.0.0.0 --port 8000`
- Start/stop via scripts: `scripts\start_server.ps1` and `scripts\stop_server.ps1`

## Coding Style & Naming Conventions
- Indentation: 4 spaces.
- Type hints are required; use modern `|` unions instead of `Optional`.
- File naming: `snake_case.py` for Python, `PascalCase` for classes.
- Each Python file must start with `# -*- coding: UTF-8 -*-`.
- Do not use magic numbers directly; centralize constants in a shared module.
- All text files must use LF (`\n`) line endings only; no CRLF (`^M`).
- Remove all trailing whitespace in every file.

## Testing Guidelines
No automated tests are set up yet. Manual test coverage is documented in `docs/TEST_CASES.md`.

## Commit & Pull Request Guidelines
Recent history uses an emoji + conventional-style prefix, e.g.:
- `:construction: chore(scanner): updated the scanner implementation`
- `:memo: docs(aicode): add README for aicode`

Use the same pattern for consistency: `:emoji: type(scope): short summary`.
For PRs, include a short description, link related issues, and list testing done.

## Agent-Specific Instructions
If you change endpoints, update `docs/PROGRAM_DESIGN.md` and `docs/USER_GUIDE.md` together.
Every session must start by reading `Plans.md`, and every task completion must update `Plans.md`.
