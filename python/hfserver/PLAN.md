# Project Plan and Status

## Goals
- Deliver an internal HotfixServer for file upload, listing, download, and deletion.
- Enforce mode-based retention policies with configurable limits.
- Provide a modern web UI aligned with product requirements.
- Maintain clear documentation and operational scripts.

## Current Status
- Backend: asyncio-based HTTP server split into modules with retention, whitelist, and logging.
- Frontend: mode tabs, per-mode list view, size/count summary, themed UI.
- Configuration: `config.json` supports whitelist, mode retention, and log file settings.
- Scripts: start/stop/restart/status/log helpers available.
- Docs: design, user guide, and test cases present.

## Next Work (if needed)
- Validate frontend mode filtering in live data.
- Add optional fallback location for weather effects.
- Add automated tests if required.

## Changelog
- 2026-02-03: Initialized plan with current implementation status.
- 2026-02-03: Split backend into modules and expanded operational scripts.
- 2026-02-03: Updated UI slogan styling and switched font to JetBrains Mono NL.
- 2026-02-03: Removed max-age retention; keep only max-files retention.
- 2026-02-03: Added cross-platform run.bat/run.sh wrappers.
- 2026-02-03: Fixed run.bat argument handling and working directory.
- 2026-02-03: Renamed PowerShell host parameter to avoid $Host collision.
- 2026-02-03: Added detailed module docstrings and stricter type hints.
- 2026-02-03: Documented formalization recommendations and created TODO.md.
- 2026-02-03: Added UTF-8 coding headers to all Python files.
- 2026-02-03: Centralized magic numbers into hotfixserver/constants.py.
- 2026-02-03: Updated AGENTS.md coding style rules for UTF-8 header and no magic numbers.
- 2026-02-03: Enforced LF line endings and documented the rule.
- 2026-02-03: Removed trailing whitespace and documented the rule.
