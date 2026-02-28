import { readdirSync, statSync } from "node:fs";
import { join } from "node:path";
import { safePath } from "../utils/path.js";
import type { Tool, ToolContext, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

export const listDirTool: Tool = {
  name: "list_dir",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "list_dir")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const dirPath = safePath(ctx.projectRoot, String(args.path ?? "."));

    try {
      const entries = readdirSync(dirPath);
      const lines = entries.map(name => {
        try {
          const s = statSync(join(dirPath, name));
          return s.isDirectory() ? `${name}/` : name;
        } catch {
          return name;
        }
      });
      return { success: true, output: lines.join("\n") };
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      return { success: false, output: "", error: msg };
    }
  },
};
