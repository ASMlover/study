import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { spawnSync } from "node:child_process";
import { APP_VERSION } from "../../src/version";

function currentTarget(): "win-x64" | "linux-x64" {
  return process.platform === "win32" ? "win-x64" : "linux-x64";
}

function currentBinaryName(baseName: string): string {
  return process.platform === "win32"
    ? `${baseName}-win-x64.exe`
    : `${baseName}-linux-x64`;
}

test("release artifact runs with --version", (t) => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-release-"));
  const outDir = path.join(tmpRoot, "dist");

  try {
    const scriptPath = path.resolve("build/src/release.js");
    const buildResult = spawnSync(
      process.execPath,
      [
        scriptPath,
        "--targets",
        currentTarget(),
        "--out-dir",
        outDir,
        "--binary-name",
        "minicli"
      ],
      {
        cwd: path.resolve("."),
        encoding: "utf8",
        stdio: "pipe"
      }
    );

    const buildError = buildResult.error as NodeJS.ErrnoException | undefined;
    if (buildError && buildError.code === "EPERM") {
      t.skip("subprocess execution is blocked in this environment");
      return;
    }

    const buildStderr = buildResult.stderr ?? "";
    if (buildResult.status !== 0 && /Missing release dependencies/.test(buildStderr)) {
      t.skip("release dependencies are not installed in this environment");
      return;
    }

    assert.equal(buildResult.status, 0, buildStderr);

    const binaryPath = path.join(outDir, currentBinaryName("minicli"));
    assert.equal(fs.existsSync(binaryPath), true);

    const runResult = spawnSync(binaryPath, ["--version"], {
      cwd: outDir,
      encoding: "utf8",
      stdio: "pipe"
    });

    const runError = runResult.error as NodeJS.ErrnoException | undefined;
    if (runError && runError.code === "EPERM") {
      t.skip("running packaged binary is blocked in this environment");
      return;
    }

    assert.equal(runResult.status, 0, runResult.stderr ?? "");
    assert.match((runResult.stdout ?? "").trim(), new RegExp(`^minicli ${APP_VERSION}`));
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
