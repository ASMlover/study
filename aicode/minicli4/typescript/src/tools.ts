import childProcess from "node:child_process";
import fs from "node:fs";
import path from "node:path";
import { RuntimeConfig } from "./config";

export interface ToolDefinition {
  type: "function";
  function: {
    name: string;
    description: string;
    parameters: {
      type: "object";
      properties: Record<string, unknown>;
      required?: string[];
    };
  };
}

export interface ToolResult {
  ok: boolean;
  output: string;
  requiresApproval?: boolean;
}

function withinRoot(projectRoot: string, target: string): boolean {
  const root = path.resolve(projectRoot);
  const resolved = path.resolve(projectRoot, target);
  return resolved === root || resolved.startsWith(`${root}${path.sep}`);
}

function normalizeAllowed(config: RuntimeConfig, projectRoot: string): string[] {
  return config.allowed_paths.map((entry) => path.resolve(projectRoot, entry));
}

function allowedPath(config: RuntimeConfig, projectRoot: string, target: string): boolean {
  const resolved = path.resolve(projectRoot, target);
  const allowed = normalizeAllowed(config, projectRoot);
  return allowed.some((base) => resolved === base || resolved.startsWith(`${base}${path.sep}`));
}

function parseArgs(raw: unknown): Record<string, unknown> {
  if (typeof raw === "string") {
    try {
      const parsed = JSON.parse(raw) as Record<string, unknown>;
      return parsed;
    } catch {
      return {};
    }
  }
  if (raw && typeof raw === "object") {
    return raw as Record<string, unknown>;
  }
  return {};
}

export class ToolRegistry {
  constructor(private readonly projectRoot: string, private readonly config: RuntimeConfig) {}

  definitions(): ToolDefinition[] {
    return [
      {
        type: "function",
        function: {
          name: "read_file",
          description: "Read a UTF-8 text file.",
          parameters: {
            type: "object",
            properties: { path: { type: "string" } },
            required: ["path"]
          }
        }
      },
      {
        type: "function",
        function: {
          name: "write_file",
          description: "Write UTF-8 content to file.",
          parameters: {
            type: "object",
            properties: { path: { type: "string" }, content: { type: "string" } },
            required: ["path", "content"]
          }
        }
      },
      {
        type: "function",
        function: {
          name: "list_dir",
          description: "List directory entries.",
          parameters: {
            type: "object",
            properties: { path: { type: "string" } },
            required: ["path"]
          }
        }
      },
      {
        type: "function",
        function: {
          name: "grep_text",
          description: "Search project text with regex.",
          parameters: {
            type: "object",
            properties: { pattern: { type: "string" }, path: { type: "string" } },
            required: ["pattern"]
          }
        }
      },
      {
        type: "function",
        function: {
          name: "run_shell",
          description: "Run shell command under policy.",
          parameters: {
            type: "object",
            properties: { command: { type: "string" } },
            required: ["command"]
          }
        }
      },
      {
        type: "function",
        function: {
          name: "project_tree",
          description: "Render project tree.",
          parameters: {
            type: "object",
            properties: { path: { type: "string" }, depth: { type: "integer" } }
          }
        }
      },
      {
        type: "function",
        function: {
          name: "session_export",
          description: "Export session messages.",
          parameters: {
            type: "object",
            properties: { out: { type: "string" }, format: { type: "string" } }
          }
        }
      }
    ];
  }

  listToolNames(): string[] {
    return this.definitions().map((x) => x.function.name);
  }

  execute(name: string, rawArgs: unknown, sessionMessagesJson = ""): ToolResult {
    const args = parseArgs(rawArgs);
    if (name === "read_file") {
      const p = String(args.path ?? "");
      if (!p || !withinRoot(this.projectRoot, p) || !allowedPath(this.config, this.projectRoot, p)) {
        return { ok: false, output: "path denied" };
      }
      const full = path.resolve(this.projectRoot, p);
      if (!fs.existsSync(full)) {
        return { ok: false, output: "not found" };
      }
      return { ok: true, output: fs.readFileSync(full, "utf8") };
    }

    if (name === "write_file") {
      if (this.config.safe_mode === "strict") {
        return { ok: false, output: "write requires explicit approval", requiresApproval: true };
      }
      const p = String(args.path ?? "");
      const content = String(args.content ?? "");
      if (!p || !withinRoot(this.projectRoot, p) || !allowedPath(this.config, this.projectRoot, p)) {
        return { ok: false, output: "path denied" };
      }
      const full = path.resolve(this.projectRoot, p);
      fs.mkdirSync(path.dirname(full), { recursive: true });
      fs.writeFileSync(full, content, "utf8");
      return { ok: true, output: `wrote ${p}` };
    }

    if (name === "list_dir") {
      const p = String(args.path ?? ".");
      if (!withinRoot(this.projectRoot, p) || !allowedPath(this.config, this.projectRoot, p)) {
        return { ok: false, output: "path denied" };
      }
      const full = path.resolve(this.projectRoot, p);
      if (!fs.existsSync(full)) {
        return { ok: false, output: "not found" };
      }
      const entries = fs.readdirSync(full, { withFileTypes: true }).map((e) => (e.isDirectory() ? `${e.name}/` : e.name));
      return { ok: true, output: entries.join("\n") };
    }

    if (name === "grep_text") {
      const patternRaw = String(args.pattern ?? "");
      if (!patternRaw) {
        return { ok: false, output: "missing pattern" };
      }
      const start = String(args.path ?? ".");
      if (!withinRoot(this.projectRoot, start) || !allowedPath(this.config, this.projectRoot, start)) {
        return { ok: false, output: "path denied" };
      }
      const regex = new RegExp(patternRaw, "i");
      const root = path.resolve(this.projectRoot, start);
      const files: string[] = [];
      const walk = (dir: string): void => {
        for (const item of fs.readdirSync(dir, { withFileTypes: true })) {
          if (item.name === ".git" || item.name === "node_modules" || item.name === ".minicli4") {
            continue;
          }
          const full = path.join(dir, item.name);
          if (item.isDirectory()) {
            walk(full);
          } else {
            files.push(full);
          }
        }
      };
      walk(root);
      const matches: string[] = [];
      for (const f of files.slice(0, 500)) {
        let text = "";
        try {
          text = fs.readFileSync(f, "utf8");
        } catch {
          continue;
        }
        const lines = text.split(/\r?\n/);
        for (let i = 0; i < lines.length; i += 1) {
          if (regex.test(lines[i])) {
            matches.push(`${path.relative(this.projectRoot, f)}:${i + 1}: ${lines[i]}`);
            if (matches.length >= 50) {
              return { ok: true, output: matches.join("\n") };
            }
          }
        }
      }
      return { ok: true, output: matches.join("\n") || "no matches" };
    }

    if (name === "run_shell") {
      const command = String(args.command ?? "").trim();
      if (!command) {
        return { ok: false, output: "missing command" };
      }
      const head = command.split(/\s+/)[0];
      if (this.config.safe_mode === "strict" && !this.config.shell_allowlist.includes(head)) {
        return { ok: false, output: `command blocked in strict mode: ${head}`, requiresApproval: true };
      }
      const proc = childProcess.spawnSync(command, {
        shell: true,
        cwd: this.projectRoot,
        encoding: "utf8",
        timeout: Math.min(this.config.timeout_ms, 30_000)
      });
      const output = [proc.stdout ?? "", proc.stderr ?? ""].join("\n").trim();
      return { ok: proc.status === 0, output: output || `(exit ${proc.status ?? -1})` };
    }

    if (name === "project_tree") {
      const p = String(args.path ?? ".");
      const depth = Number(args.depth ?? 3);
      if (!withinRoot(this.projectRoot, p) || !allowedPath(this.config, this.projectRoot, p)) {
        return { ok: false, output: "path denied" };
      }
      const root = path.resolve(this.projectRoot, p);
      const lines: string[] = [];
      const visit = (dir: string, prefix: string, d: number): void => {
        if (d < 0) {
          return;
        }
        const entries = fs.readdirSync(dir, { withFileTypes: true }).slice(0, 50);
        for (const e of entries) {
          const line = `${prefix}${e.isDirectory() ? "[D]" : "[F]"} ${e.name}`;
          lines.push(line);
          if (e.isDirectory()) {
            visit(path.join(dir, e.name), `${prefix}  `, d - 1);
          }
        }
      };
      visit(root, "", Number.isFinite(depth) ? depth : 3);
      return { ok: true, output: lines.join("\n") };
    }

    if (name === "session_export") {
      const out = String(args.out ?? ".minicli4/sessions/export.json");
      if (!withinRoot(this.projectRoot, out)) {
        return { ok: false, output: "path denied" };
      }
      const full = path.resolve(this.projectRoot, out);
      fs.mkdirSync(path.dirname(full), { recursive: true });
      fs.writeFileSync(full, sessionMessagesJson, "utf8");
      return { ok: true, output: `exported ${out}` };
    }

    return { ok: false, output: `unknown tool: ${name}` };
  }
}
