# Plans

## Project Goal

Build a standalone AI Agent CLI with Claude-Code-like TUI interaction and GLM-5 support.

## Milestones

- [x] M1: Bootstrap independent project structure
- [x] M2: Implement agent main loop and command routing
- [x] M3: Implement streaming model client and spinner
- [x] M4: Implement tool schemas, execution, and permission confirmation
- [x] M5: Implement TODO, task orchestration, and background jobs
- [x] M6: Implement sub-agent and skill support
- [x] M7: Implement context compaction and context status command
- [x] M8: Improve TUI with multi-color output and enhanced spinner

## Active Improvements

- [ ] A1: Add richer fullscreen TUI layout (panes, keybindings)
- [ ] A2: Add configurable permission policies (allowlists, session grants)
- [ ] A3: Add persistent session/history storage
- [ ] A4: Add tests for command routing and tool loop edge cases
- [ ] A5: Add provider abstraction for multiple model backends

## Next Execution Order

1. A4: Add automated tests for current stable behavior.
2. A2: Add policy-driven permission engine.
3. A1: Upgrade to fullscreen TUI without breaking existing command flow.

