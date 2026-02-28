import { readdirSync, statSync } from "node:fs";
import { join, relative } from "node:path";
import { safePath } from "../utils/path.js";
import type { Tool, ToolContext, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

function globToRegex(pattern: string): RegExp {
  let re = "";
  let i = 0;
  while (i < pattern.length) {
    const c = pattern[i];
    if (c === "*" && pattern[i + 1] === "*") {
      re += ".*";
      i += pattern[i + 2] === "/" ? 3 : 2;
    } else if (c === "*") {
      re += "[^/]*";
      i++;
    } else if (c === "?") {
      re += "[^/]";
      i++;
    } else if (c === ".") {
      re += "\\.";
      i++;
    } else {
      re += c;
      i++;
    }
  }
  return new RegExp(`^${re}$`);
}

function walkAll(dir: string, base: string, results: string[] = []): string[] {
  try {
    const entries = readdirSync(dir);
    for (const entry of entries) {
      if (entry.startsWith(".") || entry === "node_modules" || entry === "dist") continue;
      const full = join(dir, entry);
      try {
        const s = statSync(full);
        const rel = relative(base, full).replace(/\\/g, "/");
        if (s.isDirectory()) {
          results.push(rel + "/");
          walkAll(full, base, results);
        } else {
          results.push(rel);
        }
      } catch { /* skip */ }
    }
  } catch { /* skip */ }
  return results;
}

export const globTool: Tool = {
  name: "glob",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "glob")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const basePath = safePath(ctx.projectRoot, String(args.path ?? "."));
    const pattern = String(args.pattern);

    try {
      const regex = globToRegex(pattern);
      const allFiles = walkAll(basePath, basePath);
      const matches = allFiles.filter(f => regex.test(f));

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
