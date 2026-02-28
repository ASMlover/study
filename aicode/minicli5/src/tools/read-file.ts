import { readFileSync } from "node:fs";
import { safePath } from "../utils/path.js";
import type { Tool, ToolContext, ToolResult, ToolDefinition } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

export const readFileTool: Tool = {
  name: "read_file",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "read_file")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const filePath = safePath(ctx.projectRoot, String(args.path));
    const offset = args.offset ? parseInt(String(args.offset), 10) : undefined;
    const limit = args.limit ? parseInt(String(args.limit), 10) : undefined;

    try {
      const content = readFileSync(filePath, "utf-8");
      let lines = content.split("\n");

      if (offset !== undefined) {
        lines = lines.slice(Math.max(0, offset - 1));
      }
      if (limit !== undefined) {
        lines = lines.slice(0, limit);
      }

      const startLine = offset ?? 1;
      const numbered = lines.map((line, i) => `${String(startLine + i).padStart(4)}  ${line}`).join("\n");

      return { success: true, output: numbered };
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      return { success: false, output: "", error: msg };
    }
  },
};
