import test from "node:test";
import assert from "node:assert/strict";
import { COMMANDS, runSlash } from "../../src/commands";
import { DEFAULT_CONFIG } from "../../src/config";
import { SessionStore } from "../../src/session";
import { ToolRegistry } from "../../src/tools";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";

test("command baseline has 30 commands", () => {
  assert.equal(COMMANDS.length, 30);
  assert.ok(COMMANDS.includes("/agents"));
  assert.ok(COMMANDS.includes("/approve"));
});

test("/session new switches session", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-cmd-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  let current = sessions.create("default");
  const lines: string[] = [];

  const keep = runSlash(
    {
      projectRoot: root,
      config: DEFAULT_CONFIG,
      session: current,
      sessions,
      tools: new ToolRegistry(root, DEFAULT_CONFIG),
      setConfig: () => {},
      setSession: (next) => {
        current = next;
      },
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/session new demo"
  );

  assert.equal(keep, true);
  assert.equal(current.session_id, "demo");
  assert.match(lines.join("\n"), /session created/);
});
