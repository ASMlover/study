import { writeFileSync, mkdirSync, existsSync } from "node:fs";
import { dirname } from "node:path";
import { safePath } from "../utils/path.js";
import type { Tool, ToolContext, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";

export const writeFileTool: Tool = {
  name: "write_file",
  definition: TOOL_DEFINITIONS.find(d => d.function.name === "write_file")!,

  async execute(args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const filePath = safePath(ctx.projectRoot, String(args.path));
    const content = String(args.content);
    const exists = existsSync(filePath);

    if (ctx.approve) {
      const action = exists ? "overwrite" : "create";
      const detail = `${action} ${args.path} (${content.length} chars, ${content.split("\n").length} lines)`;
      // Pass first few lines as preview context
      const preview = content.split("\n").slice(0, 12).join("\n");
      const approved = await ctx.approve("write_file", detail, preview);
      if (!approved) {
        return { success: false, output: "", error: "User denied write operation" };
      }
    }

    try {
      mkdirSync(dirname(filePath), { recursive: true });
      writeFileSync(filePath, content, "utf-8");
      return { success: true, output: `Wrote ${content.length} chars to ${args.path}` };
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      return { success: false, output: "", error: msg };
    }
  },
};
