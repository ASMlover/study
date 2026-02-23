import test from "node:test";
import assert from "node:assert/strict";
import path from "node:path";
import {
  formatConfigIssue,
  loadConfigFile,
  loadMergedConfig,
  loadRuntimeConfig,
  maskSecret,
  parseConfig,
  resetManagedValueInProjectConfig,
  resolveGlobalConfigPath,
  resolveProjectConfigPath,
  saveApiKeyToGlobalConfig,
  saveManagedValueToProjectConfig,
  saveModelToProjectConfig,
  validateAndNormalizeConfig
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

test("validateAndNormalizeConfig reports missing required field", () => {
  const validated = validateAndNormalizeConfig({ timeoutMs: 1000 });
  assert.equal(validated.issues.some((x) => x.code === "missing_required"), true);
  assert.equal(validated.config.model, "glm-4");
});

test("validateAndNormalizeConfig reports type mismatch", () => {
  const validated = validateAndNormalizeConfig({
    model: "glm-4",
    timeoutMs: "fast"
  });
  assert.equal(validated.issues.some((x) => x.path === "timeoutMs"), true);
  assert.equal(validated.config.timeoutMs, 30000);
});

test("validateAndNormalizeConfig allows arbitrary non-empty model name", () => {
  const validated = validateAndNormalizeConfig({
    model: "gpt-4.1"
  });
  assert.deepEqual(validated.issues, []);
  assert.equal(validated.config.model, "gpt-4.1");
});

test("validateAndNormalizeConfig rejects empty model name", () => {
  const validated = validateAndNormalizeConfig({
    model: "   "
  });
  assert.equal(validated.issues.some((x) => x.path === "model"), true);
  assert.equal(validated.config.model, "glm-4");
});

test("validateAndNormalizeConfig reports unknown field", () => {
  const validated = validateAndNormalizeConfig({
    model: "glm-4",
    surprise: true
  });
  assert.equal(validated.issues.some((x) => x.code === "unknown_field"), true);
});

test("validateAndNormalizeConfig injects defaults", () => {
  const validated = validateAndNormalizeConfig({
    model: "glm-4-air"
  });
  assert.deepEqual(validated.issues, []);
  assert.equal(validated.config.model, "glm-4-air");
  assert.equal(validated.config.timeoutMs, 30000);
});

test("validateAndNormalizeConfig reads apiKey when valid", () => {
  const validated = validateAndNormalizeConfig({
    model: "glm-4",
    apiKey: "secret-key"
  });

  assert.equal(validated.issues.length, 0);
  assert.equal(validated.config.apiKey, "secret-key");
});

test("validateAndNormalizeConfig reports empty apiKey", () => {
  const validated = validateAndNormalizeConfig({
    model: "glm-4",
    apiKey: "   "
  });

  assert.equal(validated.issues.some((x) => x.path === "apiKey"), true);
});

test("formatConfigIssue returns readable message", () => {
  const message = formatConfigIssue({
    code: "type_mismatch",
    path: "model",
    message: "Field \"model\" cannot be empty."
  });
  assert.match(message, /\[config:type_mismatch\]/);
  assert.match(message, /model/);
});

test("loadRuntimeConfig keeps startup resilient on parse error", () => {
  const brokenPath = path.join("CWD", ".minicli", "config.json");
  const loaded = loadRuntimeConfig({
    homeDir: "HOME",
    cwd: "CWD",
    readFileSync: ((filePath: string) => {
      if (filePath === brokenPath) {
        throw new Error("Unexpected token } in JSON");
      }
      const e = new Error("missing") as NodeJS.ErrnoException;
      e.code = "ENOENT";
      throw e;
    }) as typeof import("node:fs").readFileSync
  });

  assert.equal(loaded.config.model, "glm-4");
  assert.equal(loaded.config.timeoutMs, 30000);
  assert.equal(loaded.issues.some((x) => x.code === "parse_error"), true);
});

test("saveApiKeyToGlobalConfig persists and reloads key", () => {
  const files = new Map<string, string>();
  const dirs: string[] = [];
  const globalPath = path.join("HOME", ".minicli", "config.json");

  saveApiKeyToGlobalConfig(globalPath, "sk-12345678", {
    readFileSync: (() => {
      const error = new Error("missing") as NodeJS.ErrnoException;
      error.code = "ENOENT";
      throw error;
    }) as typeof import("node:fs").readFileSync,
    mkdirSync: ((dirPath: string) => {
      dirs.push(dirPath);
    }) as typeof import("node:fs").mkdirSync,
    writeFileSync: ((filePath: string, content: string) => {
      files.set(filePath, content);
    }) as typeof import("node:fs").writeFileSync
  });

  assert.equal(dirs.length, 1);
  assert.equal(files.has(globalPath), true);
  const saved = JSON.parse(String(files.get(globalPath))) as { apiKey: string };
  assert.equal(saved.apiKey, "sk-12345678");
});

test("saveApiKeyToGlobalConfig rejects empty key", () => {
  assert.throws(
    () => saveApiKeyToGlobalConfig("C:/x/.minicli/config.json", "   "),
    /API key cannot be empty/
  );
});

test("maskSecret masks key value", () => {
  assert.equal(maskSecret("abcd"), "****");
  assert.equal(maskSecret("sk-12345678"), "*******5678");
});

test("saveModelToProjectConfig trims and saves arbitrary model name", () => {
  const files = new Map<string, string>();
  const projectPath = path.join("CWD", ".minicli", "config.json");

  const saved = saveModelToProjectConfig(projectPath, "  gpt-4.1  ", {
    readFileSync: ((_: string) =>
      JSON.stringify({ timeoutMs: 1000 })) as typeof import("node:fs").readFileSync,
    mkdirSync: (() => {}) as typeof import("node:fs").mkdirSync,
    writeFileSync: ((filePath: string, content: string) => {
      files.set(filePath, content);
    }) as typeof import("node:fs").writeFileSync
  });

  assert.equal(saved, "gpt-4.1");
  const parsed = JSON.parse(String(files.get(projectPath))) as {
    model: string;
    timeoutMs: number;
  };
  assert.equal(parsed.model, "gpt-4.1");
  assert.equal(parsed.timeoutMs, 1000);
});

test("saveModelToProjectConfig rejects empty model name", () => {
  assert.throws(
    () => saveModelToProjectConfig("C:/x/.minicli/config.json", "   "),
    /Model name cannot be empty/
  );
});

test("validateAndNormalizeConfig reads extended numeric runtime config keys", () => {
  const validated = validateAndNormalizeConfig({
    model: "glm-4",
    runConfirmationTimeoutMs: 12000,
    requestTokenBudget: 6000
  });

  assert.equal(validated.issues.length, 0);
  assert.equal(validated.config.runConfirmationTimeoutMs, 12000);
  assert.equal(validated.config.requestTokenBudget, 6000);
});

test("saveManagedValueToProjectConfig persists value and reset removes it", () => {
  const files = new Map<string, string>();
  const projectPath = path.join("CWD", ".minicli", "config.json");

  const readFileSync = ((filePath: string) => {
    const value = files.get(filePath);
    if (value === undefined) {
      const error = new Error("missing") as NodeJS.ErrnoException;
      error.code = "ENOENT";
      throw error;
    }
    return value;
  }) as typeof import("node:fs").readFileSync;

  const writeFileSync = ((filePath: string, content: string) => {
    files.set(filePath, content);
  }) as typeof import("node:fs").writeFileSync;

  const mkdirSync = (() => {}) as typeof import("node:fs").mkdirSync;

  saveManagedValueToProjectConfig(projectPath, "requestTokenBudget", 8000, {
    readFileSync,
    writeFileSync,
    mkdirSync
  });
  const saved = JSON.parse(String(files.get(projectPath))) as {
    requestTokenBudget?: number;
  };
  assert.equal(saved.requestTokenBudget, 8000);

  resetManagedValueInProjectConfig(projectPath, "requestTokenBudget", {
    readFileSync,
    writeFileSync,
    mkdirSync
  });
  const reset = JSON.parse(String(files.get(projectPath))) as {
    requestTokenBudget?: number;
  };
  assert.equal(reset.requestTokenBudget, undefined);
});
