import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { loadMergedConfig } from "../../src/config";

test("global and project configs merge with project override", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-config-"));
  const homeDir = path.join(tmpRoot, "home");
  const cwd = path.join(tmpRoot, "project");
  const globalConfigDir = path.join(homeDir, ".minicli");
  const projectConfigDir = path.join(cwd, ".minicli");

  fs.mkdirSync(globalConfigDir, { recursive: true });
  fs.mkdirSync(projectConfigDir, { recursive: true });
  fs.writeFileSync(
    path.join(globalConfigDir, "config.json"),
    JSON.stringify({
      model: "global-model",
      timeoutMs: 12000,
      endpoint: "https://global.example.com"
    }),
    "utf8"
  );
  fs.writeFileSync(
    path.join(projectConfigDir, "config.json"),
    JSON.stringify({
      model: "project-model",
      endpoint: "https://project.example.com"
    }),
    "utf8"
  );

  try {
    const loaded = loadMergedConfig({ homeDir, cwd });
    assert.deepEqual(loaded.globalConfig, {
      model: "global-model",
      timeoutMs: 12000,
      endpoint: "https://global.example.com"
    });
    assert.deepEqual(loaded.projectConfig, {
      model: "project-model",
      endpoint: "https://project.example.com"
    });
    assert.deepEqual(loaded.mergedConfig, {
      model: "project-model",
      timeoutMs: 12000,
      endpoint: "https://project.example.com"
    });
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
