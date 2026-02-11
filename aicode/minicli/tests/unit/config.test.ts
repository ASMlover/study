import test from "node:test";
import assert from "node:assert/strict";
import path from "node:path";
import {
  loadConfigFile,
  loadMergedConfig,
  parseConfig,
  resolveGlobalConfigPath,
  resolveProjectConfigPath
} from "../../src/config";

test("resolves global config path", () => {
  const homeDir = path.join("C:", "Users", "tester");
  const resolved = resolveGlobalConfigPath(homeDir);
  assert.equal(
    resolved,
    path.join("C:", "Users", "tester", ".minicli", "config.json")
  );
});

test("resolves project config path", () => {
  const cwd = path.join("C:", "workspace", "project");
  const resolved = resolveProjectConfigPath(cwd);
  assert.equal(
    resolved,
    path.join("C:", "workspace", "project", ".minicli", "config.json")
  );
});

test("project config overrides global config", () => {
  const files = new Map<string, string>([
    [
      path.join("HOME", ".minicli", "config.json"),
      JSON.stringify({ model: "global-model", timeoutMs: 15000, keep: true })
    ],
    [
      path.join("CWD", ".minicli", "config.json"),
      JSON.stringify({ model: "project-model" })
    ]
  ]);

  const loaded = loadMergedConfig({
    homeDir: "HOME",
    cwd: "CWD",
    readFileSync: ((filePath: string) => {
      const value = files.get(filePath);
      if (value === undefined) {
        const error = new Error("missing") as NodeJS.ErrnoException;
        error.code = "ENOENT";
        throw error;
      }
      return value;
    }) as typeof import("node:fs").readFileSync
  });

  assert.deepEqual(loaded.mergedConfig, {
    model: "project-model",
    timeoutMs: 15000,
    keep: true
  });
});

test("missing config files gracefully fallback to empty config", () => {
  const loaded = loadMergedConfig({
    homeDir: "HOME",
    cwd: "CWD",
    readFileSync: (() => {
      const error = new Error("missing") as NodeJS.ErrnoException;
      error.code = "ENOENT";
      throw error;
    }) as typeof import("node:fs").readFileSync
  });

  assert.deepEqual(loaded.globalConfig, {});
  assert.deepEqual(loaded.projectConfig, {});
  assert.deepEqual(loaded.mergedConfig, {});
});

test("empty config content returns empty object", () => {
  assert.deepEqual(parseConfig(""), {});
  assert.deepEqual(parseConfig(" \n\t "), {});
});

test("parses json config with comments", () => {
  const parsed = parseConfig(`
# top-level comment
{
  // line comment
  "model": "glm-4",
  "timeoutMs": 20000, /* block comment */
  "note": "keep // in string"
}
`);

  assert.deepEqual(parsed, {
    model: "glm-4",
    timeoutMs: 20000,
    note: "keep // in string"
  });
});

test("loadConfigFile returns empty for missing file", () => {
  const loaded = loadConfigFile(
    path.join("none", ".minicli", "config.json"),
    (() => {
      const error = new Error("missing") as NodeJS.ErrnoException;
      error.code = "ENOENT";
      throw error;
    }) as typeof import("node:fs").readFileSync
  );

  assert.deepEqual(loaded, {});
});
