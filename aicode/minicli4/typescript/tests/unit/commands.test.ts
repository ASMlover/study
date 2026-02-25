import test from "node:test";
import assert from "node:assert/strict";
import { COMMANDS, runSlash } from "../../src/commands";
import { DEFAULT_CONFIG, RuntimeConfig } from "../../src/config";
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

test("/help shows grouped command overview", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-help-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: DEFAULT_CONFIG,
      session: current,
      sessions,
      tools: new ToolRegistry(root, DEFAULT_CONFIG),
      setConfig: () => {},
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/help"
  );

  const text = lines.join("\n");
  assert.match(text, /MiniCLI4 Help/);
  assert.match(text, /Usage: \/help \[command\]/);
  assert.match(text, /Core:/);
  assert.match(text, /Session:/);
  assert.match(text, /Tools:/);
  assert.match(text, /System:/);
  assert.match(text, /\/help\s+Show slash command help\./);
  assert.match(text, /\/run\s+Run shell command under safety policy\./);
  assert.match(text, /Use \/help <command> for command details\./);
});

test("/help <command> shows command details", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-help-item-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: DEFAULT_CONFIG,
      session: current,
      sessions,
      tools: new ToolRegistry(root, DEFAULT_CONFIG),
      setConfig: () => {},
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/help config"
  );

  const text = lines.join("\n");
  assert.match(text, /\/config - Read or update local project configuration\./);
  assert.match(text, /Usage: \/config <get\|set\|list\|reset> \[key\] \[value\]/);
  assert.match(text, /Subcommands: get, set, list, reset/);
});

test("/status prints product-style status card", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-status-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: DEFAULT_CONFIG,
      session: current,
      sessions,
      tools: new ToolRegistry(root, DEFAULT_CONFIG),
      setConfig: () => {},
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/status"
  );

  const text = lines.join("\n");
  assert.match(text, /\[STATUS\]/);
  assert.match(text, /session\s+: default/);
  assert.match(text, /model\s+: glm-5/);
  assert.match(text, /safe_mode\s+: strict/);
  assert.match(text, /theme\s+: (dark|light)/);
  assert.match(text, /motion\s+: (full|minimal)/);
});

test("/doctor prints product-style diagnostics card", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-doctor-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: DEFAULT_CONFIG,
      session: current,
      sessions,
      tools: new ToolRegistry(root, DEFAULT_CONFIG),
      setConfig: () => {},
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/doctor"
  );

  const text = lines.join("\n");
  assert.match(text, /\[DOCTOR\]/);
  assert.match(text, /runtime\s+: reachable/);
  assert.match(text, /model_lock\s+: ok/);
  assert.match(text, /api_key\s+: missing/);
});

test("/config set theme updates config", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-theme-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  let activeConfig: RuntimeConfig = { ...DEFAULT_CONFIG, theme: "dark" };
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: activeConfig,
      session: current,
      sessions,
      tools: new ToolRegistry(root, activeConfig),
      setConfig: (next) => {
        activeConfig = next;
      },
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/config set theme light"
  );

  assert.equal(activeConfig.theme, "light");
  assert.match(lines.join("\n"), /config updated/);
});

test("/config set motion updates config", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-motion-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  let activeConfig: RuntimeConfig = { ...DEFAULT_CONFIG, motion: "full" };
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: activeConfig,
      session: current,
      sessions,
      tools: new ToolRegistry(root, activeConfig),
      setConfig: (next) => {
        activeConfig = next;
      },
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/config set motion minimal"
  );

  assert.equal(activeConfig.motion, "minimal");
  assert.match(lines.join("\n"), /config updated/);
});

test("/context prints usage percentage for archive/clear decisions", () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "minicli4-ts-context-"));
  const state = path.join(root, ".minicli4");
  fs.mkdirSync(path.join(state, "sessions"), { recursive: true });
  const sessions = new SessionStore(state);
  const current = sessions.create("default");
  const now = new Date().toISOString();
  current.messages.push(
    { role: "user", content: "a".repeat(400), created_at: now },
    { role: "assistant", content: "b".repeat(400), created_at: now }
  );
  const lines: string[] = [];

  runSlash(
    {
      projectRoot: root,
      config: { ...DEFAULT_CONFIG, max_tokens: 1000 },
      session: current,
      sessions,
      tools: new ToolRegistry(root, DEFAULT_CONFIG),
      setConfig: () => {},
      setSession: () => {},
      out: (line) => lines.push(line),
      setPendingApproval: () => {},
      clearPendingApproval: () => {},
      pendingApproval: () => ""
    },
    "/context"
  );

  const text = lines.join("\n");
  assert.match(text, /messages=2/);
  assert.match(text, /tokens~=200/);
  assert.match(text, /context_usage~=20% \(200\/1000\)/);
});

