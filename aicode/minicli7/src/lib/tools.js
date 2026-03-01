import fs from "node:fs/promises";
import path from "node:path";
import { exec as execCb } from "node:child_process";
import { promisify } from "node:util";

const exec = promisify(execCb);

function safeResolve(cwd, target) {
  const resolved = path.resolve(cwd, target || ".");
  if (!resolved.startsWith(path.resolve(cwd))) {
    throw new Error("Path is outside project directory");
  }
  return resolved;
}

export function buildToolSchemas() {
  return [
    {
      type: "function",
      function: {
        name: "list_files",
        description: "List files in a directory under the project.",
        parameters: {
          type: "object",
          properties: {
            path: { type: "string", description: "Relative path in project" },
          },
          required: ["path"],
        },
      },
    },
    {
      type: "function",
      function: {
        name: "read_file",
        description: "Read a UTF-8 text file under the project.",
        parameters: {
          type: "object",
          properties: {
            path: { type: "string" },
          },
          required: ["path"],
        },
      },
    },
    {
      type: "function",
      function: {
        name: "write_file",
        description: "Write UTF-8 content to a file under the project.",
        parameters: {
          type: "object",
          properties: {
            path: { type: "string" },
            content: { type: "string" },
          },
          required: ["path", "content"],
        },
      },
    },
    {
      type: "function",
      function: {
        name: "grep",
        description: "Search text in files under project root.",
        parameters: {
          type: "object",
          properties: {
            pattern: { type: "string" },
            path: { type: "string" },
          },
          required: ["pattern"],
        },
      },
    },
    {
      type: "function",
      function: {
        name: "run_shell",
        description: "Run a shell command in the project directory.",
        parameters: {
          type: "object",
          properties: {
            command: { type: "string" },
            background: { type: "boolean" },
          },
          required: ["command"],
        },
      },
    },
    {
      type: "function",
      function: {
        name: "spawn_agent",
        description: "Spawn a sub-agent to solve a scoped goal.",
        parameters: {
          type: "object",
          properties: {
            goal: { type: "string" },
            maxSteps: { type: "number" },
          },
          required: ["goal"],
        },
      },
    },
    {
      type: "function",
      function: {
        name: "manage_todo",
        description: "Add, complete, or list TODO items.",
        parameters: {
          type: "object",
          properties: {
            action: { type: "string", enum: ["add", "done", "list"] },
            text: { type: "string" },
            id: { type: "number" },
          },
          required: ["action"],
        },
      },
    },
  ];
}

export class ToolExecutor {
  constructor({ cwd, background, store, subAgentRunner }) {
    this.cwd = cwd;
    this.background = background;
    this.store = store;
    this.subAgentRunner = subAgentRunner;
  }

  async execute(name, args) {
    switch (name) {
      case "list_files": {
        const abs = safeResolve(this.cwd, args.path);
        const entries = await fs.readdir(abs, { withFileTypes: true });
        return entries.map((e) => `${e.isDirectory() ? "[D]" : "[F]"} ${e.name}`).join("\n");
      }
      case "read_file": {
        const abs = safeResolve(this.cwd, args.path);
        const text = await fs.readFile(abs, "utf8");
        return text.slice(0, 20000);
      }
      case "write_file": {
        const abs = safeResolve(this.cwd, args.path);
        await fs.mkdir(path.dirname(abs), { recursive: true });
        await fs.writeFile(abs, args.content, "utf8");
        return `Wrote ${args.path} (${args.content.length} chars)`;
      }
      case "grep": {
        const rel = args.path || ".";
        const abs = safeResolve(this.cwd, rel);
        const cmd = `rg -n --no-heading --color never ${quote(args.pattern)} ${quote(abs)}`;
        const { stdout } = await exec(cmd, { cwd: this.cwd, maxBuffer: 1024 * 1024 * 8 });
        return stdout.trim() || "No matches";
      }
      case "run_shell": {
        if (args.background) {
          const job = this.background.start(args.command);
          return `Started background job #${job.id} pid=${job.pid}`;
        }
        const { stdout, stderr } = await exec(args.command, {
          cwd: this.cwd,
          maxBuffer: 1024 * 1024 * 8,
        });
        const text = [stdout, stderr].filter(Boolean).join("\n").trim();
        return text || "Command finished with no output";
      }
      case "spawn_agent": {
        return this.subAgentRunner.run(args.goal, Math.max(1, Number(args.maxSteps || 4)));
      }
      case "manage_todo": {
        if (args.action === "add") {
          const todo = this.store.addTodo(args.text || "Untitled");
          return `Added todo #${todo.id}: ${todo.text}`;
        }
        if (args.action === "done") {
          const todo = this.store.markTodoDone(Number(args.id));
          return todo ? `Done todo #${todo.id}` : "Todo not found";
        }
        return this.store
          .listTodos()
          .map((t) => `${t.done ? "[x]" : "[ ]"} #${t.id} ${t.text}`)
          .join("\n");
      }
      default:
        throw new Error(`Unknown tool: ${name}`);
    }
  }
}

function quote(value) {
  const s = String(value ?? "");
  if (/^[a-zA-Z0-9_./:\\-]+$/.test(s)) {
    return s;
  }
  return `"${s.replace(/"/g, '\\"')}"`;
}

export function parseToolArguments(raw) {
  try {
    return JSON.parse(raw || "{}");
  } catch {
    return {};
  }
}
