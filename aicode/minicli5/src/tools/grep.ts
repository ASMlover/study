import { readFileSync, readdirSync, statSync } from "node:fs";
import { join, relative } from "node:path";
import { safePath } from "../utils/path.js";
import type { Tool, ToolContext, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

function matchGlob(filename: string, pattern: string): boolean {
  const regex = pattern
    .replace(/\./g, "\\.")
    .replace(/\*/g, ".*")
    .replace(/\?/g, ".");
  return new RegExp(`^${regex}$`).test(filename);
}

function walkFiles(dir: string, globPattern?: string, results: string[] = []): string[] {
  try {
    const entries = readdirSync(dir);
    for (const entry of entries) {
      if (entry.startsWith(".") || entry === "node_modules" || entry === "dist") continue;
      const full = join(dir, entry);
      try {
        const s = statSync(full);
        if (s.isDirectory()) {
          walkFiles(full, globPattern, results);
        } else if (!globPattern || matchGlob(entry, globPattern)) {
          results.push(full);
        }
      } catch { /* skip */ }
    }
  } catch { /* skip */ }
  return results;
}

export const grepTool: Tool = {
  name: "grep",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "grep")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const searchPath = safePath(ctx.projectRoot, String(args.path ?? "."));
    const pattern = String(args.pattern);
    const glob = args.glob ? String(args.glob) : undefined;

    try {
      const regex = new RegExp(pattern, "i");
      const files = walkFiles(searchPath, glob);
      const matches: string[] = [];

      for (const file of files) {
        if (matches.length > 100) break;
        try {
          const content = readFileSync(file, "utf-8");
          const lines = content.split("\n");
          for (let i = 0; i < lines.length; i++) {
            if (regex.test(lines[i])) {
              const relPath = relative(ctx.projectRoot, file);
              matches.push(`${relPath}:${i + 1}: ${lines[i].trim()}`);
              if (matches.length > 100) break;
            }
          }
        } catch { /* skip binary/unreadable */ }
      }

      if (matches.length === 0) {
        return { success: true, output: "No matches found." };
      }
      return { success: true, output: matches.join("\n") };
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      return { success: false, output: "", error: msg };
    }
  },
};
