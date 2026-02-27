# MiniCLI4 Verification Log

## 2026-02-26 - M4 Python Parity

- Workspace: `python/`
- Command: `pytest`
- Result: `19 passed`
- Coverage focus:
  - Python CLI scaffold and entrypoint behavior.
  - Slash command baseline and command behavior parity (`30` commands).
  - Completion vector parity using shared `spec/test-vectors/completion.*.json`.
  - Provider error mapping and stream parsing behavior.
  - Multi-agent orchestration and tool loop.
  - Markdown streaming renderer behavior.
  - Append-only Python TUI status/thinking/stream render flow.

## 2026-02-27 - M5 C++ Parity

- Workspace: `cpp/`
- Command: `cmake -S cpp -B build/cpp`
- Result: `configure/generate succeeded`
- Command: `cmake --build build/cpp --config Debug`
- Result: `build succeeded`
- Command: `ctest --test-dir build/cpp -C Debug --output-on-failure`
- Result: `1/1 tests passed`
- Command: `build\\cpp\\Debug\\minicli4-cpp.exe --version`
- Result: `minicli4-cpp 0.1.0`
- Coverage focus:
  - C++ workspace scaffold with CMake library + CLI + ctest target.
  - Slash command baseline parity (`30` commands) and core semantics.
  - Completion behavior parity for command prefixes, subcommands, dynamic arguments, and path candidates.
  - Provider SSE stream chunk parsing and error envelope surface.
  - Multi-agent orchestration flow with staged callbacks and tool loop behavior.
  - Append-only C++ TUI status/thinking/stream lifecycle behavior.

## 2026-02-27 - M5 C++ FTXUI Wiring Follow-up

- Workspace: `cpp/`
- Command: `cmake -S cpp -B build/cpp`
- Result: `configured with "FTXUI not found, using ANSI fallback backend"`
- Command: `cmake --build build/cpp --config Debug`
- Result: `build succeeded`
- Command: `ctest --test-dir build/cpp -C Debug --output-on-failure`
- Result: `1/1 tests passed`
- Coverage focus:
  - Added optional FTXUI backend linking/wiring in CMake (`MINICLI4_ENABLE_FTXUI`, `find_package(ftxui)`).
  - Added runtime TUI backend selection with `MINICLI4_TUI_BACKEND` and automatic FTXUI default when available.
  - Preserved existing append-only ANSI behavior and contract-compatible method surface as fallback.
