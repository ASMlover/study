import fs from "node:fs";
import path from "node:path";
import { spawnSync } from "node:child_process";

export type ReleaseTarget = "win-x64" | "linux-x64";

export interface ReleaseBuildOptions {
  entryFile: string;
  outDir: string;
  bundleFileName: string;
  binaryBaseName: string;
  version: string;
  targets: ReleaseTarget[];
}

export interface ReleaseBuildPlan {
  bundlePath: string;
  pkgTargets: string[];
  binaryOutputPaths: string[];
  versionDefineValue: string;
}

const SUPPORTED_TARGETS: readonly ReleaseTarget[] = ["win-x64", "linux-x64"];
const DEFAULT_RELEASE_OPTIONS: ReleaseBuildOptions = {
  entryFile: "src/index.ts",
  outDir: "dist",
  bundleFileName: "minicli.bundle.cjs",
  binaryBaseName: "minicli",
  version: "0.0.0",
  targets: [...SUPPORTED_TARGETS]
};

export function parseReleaseBuildArgs(
  argv: readonly string[],
  defaults: ReleaseBuildOptions = DEFAULT_RELEASE_OPTIONS
): ReleaseBuildOptions {
  const options: ReleaseBuildOptions = {
    ...defaults,
    targets: [...defaults.targets]
  };

  for (let i = 0; i < argv.length; i += 1) {
    const token = argv[i];
    if (!token.startsWith("--")) {
      throw new Error(`Unexpected argument: ${token}`);
    }

    const next = argv[i + 1];
    if (next === undefined || next.startsWith("--")) {
      throw new Error(`Missing value for ${token}`);
    }

    switch (token) {
      case "--entry":
        options.entryFile = ensureNonEmpty(next, token);
        i += 1;
        break;
      case "--out-dir":
        options.outDir = ensureNonEmpty(next, token);
        i += 1;
        break;
      case "--bundle-name":
        options.bundleFileName = ensureNonEmpty(next, token);
        i += 1;
        break;
      case "--binary-name":
        options.binaryBaseName = ensureNonEmpty(next, token);
        i += 1;
        break;
      case "--version":
        options.version = ensureNonEmpty(next, token);
        i += 1;
        break;
      case "--targets":
        options.targets = parseReleaseTargets(next);
        i += 1;
        break;
      default:
        throw new Error(`Unknown option: ${token}`);
    }
  }

  validateEntryPathShape(options.entryFile);
  return options;
}

function ensureNonEmpty(value: string, optionName: string): string {
  const normalized = value.trim();
  if (normalized.length === 0) {
    throw new Error(`Empty value for ${optionName}`);
  }
  return normalized;
}

export function parseReleaseTargets(value: string): ReleaseTarget[] {
  const parsed = value
    .split(",")
    .map((target) => target.trim())
    .filter((target) => target.length > 0);

  if (parsed.length === 0) {
    throw new Error("At least one release target is required.");
  }

  const seen = new Set<ReleaseTarget>();
  const targets: ReleaseTarget[] = [];
  for (const target of parsed) {
    if (!SUPPORTED_TARGETS.includes(target as ReleaseTarget)) {
      throw new Error(`Unsupported release target: ${target}`);
    }
    const typedTarget = target as ReleaseTarget;
    if (!seen.has(typedTarget)) {
      seen.add(typedTarget);
      targets.push(typedTarget);
    }
  }

  return targets;
}

export function validateEntryPathShape(entryFile: string): void {
  if (!entryFile.endsWith(".ts") && !entryFile.endsWith(".js")) {
    throw new Error(`Release entry must be .ts or .js: ${entryFile}`);
  }
}

export function ensureReleaseEntrypointExists(
  entryPath: string,
  exists: (candidate: string) => boolean = fs.existsSync
): void {
  if (!exists(entryPath)) {
    throw new Error(`Release entry not found: ${entryPath}`);
  }
}

export function resolvePackageOutputName(
  binaryBaseName: string,
  target: ReleaseTarget
): string {
  const suffix = target === "win-x64" ? "win-x64.exe" : "linux-x64";
  return `${binaryBaseName}-${suffix}`;
}

export function resolvePkgTarget(target: ReleaseTarget): string {
  return target === "win-x64" ? "node22-win-x64" : "node22-linux-x64";
}

export function findMissingPackagingDependencies(
  resolver: (moduleName: string) => string = require.resolve
): string[] {
  const required = ["esbuild", "pkg"];
  const missing: string[] = [];
  for (const moduleName of required) {
    try {
      resolver(`${moduleName}/package.json`);
    } catch {
      missing.push(moduleName);
    }
  }
  return missing;
}

export function formatMissingDependencyError(missing: readonly string[]): string {
  return `Missing release dependencies: ${missing.join(", ")}. Run: npm install`;
}

export function createReleaseBuildPlan(options: ReleaseBuildOptions): ReleaseBuildPlan {
  const bundlePath = path.join(options.outDir, options.bundleFileName);
  const pkgTargets = options.targets.map((target) => resolvePkgTarget(target));
  const binaryOutputPaths = options.targets.map((target) =>
    path.join(options.outDir, resolvePackageOutputName(options.binaryBaseName, target))
  );
  return {
    bundlePath,
    pkgTargets,
    binaryOutputPaths,
    versionDefineValue: JSON.stringify(options.version)
  };
}

function resolvePkgBinPath(
  resolver: (moduleName: string) => string = require.resolve,
  readFile: (filePath: string, encoding: BufferEncoding) => string =
    (filePath, encoding) => fs.readFileSync(filePath, encoding)
): string {
  const packageJsonPath = resolver("pkg/package.json");
  const packageJsonRaw = readFile(packageJsonPath, "utf8");
  const packageJson = JSON.parse(packageJsonRaw) as { bin?: string | Record<string, string> };
  const packageRoot = path.dirname(packageJsonPath);

  if (typeof packageJson.bin === "string") {
    return path.resolve(packageRoot, packageJson.bin);
  }

  if (packageJson.bin && typeof packageJson.bin === "object") {
    const first = Object.values(packageJson.bin)[0];
    if (typeof first === "string") {
      return path.resolve(packageRoot, first);
    }
  }

  throw new Error("Cannot resolve pkg CLI binary path.");
}

export interface ReleaseBuildResult {
  bundlePath: string;
  binaryOutputPaths: string[];
}

export function runReleaseBuild(options: ReleaseBuildOptions): ReleaseBuildResult {
  const missing = findMissingPackagingDependencies();
  if (missing.length > 0) {
    throw new Error(formatMissingDependencyError(missing));
  }

  validateEntryPathShape(options.entryFile);
  ensureReleaseEntrypointExists(options.entryFile);

  const plan = createReleaseBuildPlan(options);
  fs.mkdirSync(options.outDir, { recursive: true });

  const esbuildPath = require.resolve("esbuild/bin/esbuild");
  const esbuildArgs = [
    options.entryFile,
    "--bundle",
    "--platform=node",
    "--format=cjs",
    "--target=node22",
    `--outfile=${plan.bundlePath}`,
    "--banner:js=#!/usr/bin/env node",
    `--define:process.env.MINICLI_APP_VERSION=${plan.versionDefineValue}`
  ];

  const bundleResult = spawnSync(process.execPath, [esbuildPath, ...esbuildArgs], {
    encoding: "utf8",
    stdio: "pipe"
  });

  if (bundleResult.status !== 0) {
    const stderr = bundleResult.stderr?.toString() ?? "";
    throw new Error(`[release:esbuild] ${stderr.trim()}`);
  }

  const pkgBinPath = resolvePkgBinPath();
  const pkgArgs = [
    plan.bundlePath,
    "--targets",
    plan.pkgTargets.join(","),
    "--out-path",
    options.outDir,
    "--output",
    options.binaryBaseName
  ];
  const pkgResult = spawnSync(process.execPath, [pkgBinPath, ...pkgArgs], {
    encoding: "utf8",
    stdio: "pipe"
  });

  if (pkgResult.status !== 0) {
    const stderr = pkgResult.stderr?.toString() ?? "";
    throw new Error(`[release:pkg] ${stderr.trim()}`);
  }

  return {
    bundlePath: plan.bundlePath,
    binaryOutputPaths: plan.binaryOutputPaths
  };
}

export function readPackageVersion(packageJsonPath = "package.json"): string {
  const raw = fs.readFileSync(packageJsonPath, "utf8");
  const parsed = JSON.parse(raw) as { version?: string };
  if (!parsed.version || parsed.version.trim().length === 0) {
    throw new Error(`package.json version is missing: ${packageJsonPath}`);
  }
  return parsed.version.trim();
}

export function runReleaseBuildCli(argv: readonly string[]): number {
  try {
    const defaults: ReleaseBuildOptions = {
      ...DEFAULT_RELEASE_OPTIONS,
      version: readPackageVersion()
    };
    const options = parseReleaseBuildArgs(argv, defaults);
    const result = runReleaseBuild(options);
    process.stdout.write(`[release] bundle: ${result.bundlePath}\n`);
    for (const filePath of result.binaryOutputPaths) {
      process.stdout.write(`[release] binary: ${filePath}\n`);
    }
    return 0;
  } catch (error) {
    const e = error as Error;
    process.stderr.write(`${e.message}\n`);
    return 1;
  }
}

if (require.main === module) {
  process.exitCode = runReleaseBuildCli(process.argv.slice(2));
}
