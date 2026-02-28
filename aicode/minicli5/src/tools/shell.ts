import { execSync } from "node:child_process";
import type { Tool, ToolContext, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

// Commands considered safe (read-only, no side effects)
const SAFE_PREFIXES = [
  "git status", "git log", "git diff", "git branch", "git show",
  "ls", "cat", "head", "tail", "wc", "file", "which", "where",
  "echo", "date", "pwd", "whoami",
  "node --version", "npm --version", "npx --version",
  "tsc --version", "python --version",
];

function isSafeCommand(cmd: string): boolean {
  const trimmed = cmd.trim();
  return SAFE_PREFIXES.some(p => trimmed === p || trimmed.startsWith(p + " "));
}

export const shellTool: Tool = {
  name: "shell",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "shell")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const command = String(args.command);
    const timeout = args.timeout ? parseInt(String(args.timeout), 10) : 30000;

    // Only require approval for non-safe commands
    if (ctx.approve && !isSafeCommand(command)) {
      const approved = await ctx.approve(
        "shell",
        command,
        `cwd: ${ctx.projectRoot}\ntimeout: ${timeout}ms`,
      );
      if (!approved) {
        return { success: false, output: "", error: "User denied shell execution" };
      }
    }

    try {
      const output = execSync(command, {
        cwd: ctx.projectRoot,
        timeout,
        encoding: "utf-8",
        maxBuffer: 1024 * 1024,
        stdio: ["pipe", "pipe", "pipe"],
      });
      return { success: true, output: output.trim() };
    } catch (err: unknown) {
      if (err && typeof err === "object" && "stdout" in err) {
        const e = err as { stdout?: string; stderr?: string; status?: number };
        const output = [e.stdout, e.stderr].filter(Boolean).join("\n").trim();
        return { success: false, output, error: `Exit code: ${e.status ?? 1}` };
      }
      const msg = err instanceof Error ? err.message : String(err);
      return { success: false, output: "", error: msg };
    }
  },
};
