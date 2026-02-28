import { readdirSync, statSync } from "node:fs";
import { join } from "node:path";
import { safePath } from "../utils/path.js";
import type { Tool, ToolContext, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

const SKIP = new Set(["node_modules", "dist", ".git", ".minicli5", "__pycache__", ".next"]);

function buildTree(dir: string, prefix: string, depth: number, maxDepth: number): string[] {
  if (depth > maxDepth) return [];

  const lines: string[] = [];
  let entries: string[];
  try {
    entries = readdirSync(dir);
  } catch {
    return [];
  }

  entries = entries.filter(e => !e.startsWith(".") || e === ".env.example");
  entries = entries.filter(e => !SKIP.has(e));
  entries.sort();

  for (let i = 0; i < entries.length; i++) {
    const entry = entries[i];
    const full = join(dir, entry);
    const isLast = i === entries.length - 1;
    const connector = isLast ? "└── " : "├── ";
    const childPrefix = isLast ? "    " : "│   ";

    try {
      const s = statSync(full);
      if (s.isDirectory()) {
        lines.push(`${prefix}${connector}${entry}/`);
        lines.push(...buildTree(full, prefix + childPrefix, depth + 1, maxDepth));
      } else {
        lines.push(`${prefix}${connector}${entry}`);
      }
    } catch {
      lines.push(`${prefix}${connector}${entry}`);
    }
  }

  return lines;
}

export const projectTreeTool: Tool = {
  name: "project_tree",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "project_tree")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const treePath = safePath(ctx.projectRoot, String(args.path ?? "."));
    const maxDepth = args.depth ? parseInt(String(args.depth), 10) : 3;

    try {
      const lines = buildTree(treePath, "", 0, maxDepth);
      return { success: true, output: lines.join("\n") || "(empty directory)" };
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      return { success: false, output: "", error: msg };
    }
  },
};
