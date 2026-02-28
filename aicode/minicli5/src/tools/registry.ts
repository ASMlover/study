import type { Tool, ToolContext, ToolDefinition, ToolResult } from "../types.js";
import { TOOL_DEFINITIONS } from "./definitions.js";
import { readFileTool } from "./read-file.js";
import { writeFileTool } from "./write-file.js";
import { listDirTool } from "./list-dir.js";
import { grepTool } from "./grep.js";
import { shellTool } from "./shell.js";
import { globTool } from "./glob.js";
import { projectTreeTool } from "./project-tree.js";
import { log } from "../utils/logger.js";

export class ToolRegistry {
  private tools = new Map<string, Tool>();

  constructor() {
    this.register(readFileTool);
    this.register(writeFileTool);
    this.register(listDirTool);
    this.register(grepTool);
    this.register(shellTool);
    this.register(globTool);
    this.register(projectTreeTool);
  }

  register(tool: Tool): void {
    this.tools.set(tool.name, tool);
  }

  get(name: string): Tool | undefined {
    return this.tools.get(name);
  }

  getDefinitions(filter?: string[]): ToolDefinition[] {
    if (!filter) return TOOL_DEFINITIONS;
    return TOOL_DEFINITIONS.filter(d => filter.includes(d.function.name));
  }

  list(): string[] {
    return [...this.tools.keys()];
  }

  async execute(name: string, args: Record<string, unknown>, ctx: ToolContext): Promise<ToolResult> {
    const tool = this.tools.get(name);
    if (!tool) {
      return { success: false, output: "", error: `Unknown tool: ${name}` };
    }

    try {
      log("info", `Tool execute: ${name}`, args);
      const result = await tool.execute(args, ctx);
      log("info", `Tool result: ${name}`, { success: result.success, outputLen: result.output.length });
      return result;
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : String(err);
      log("error", `Tool error: ${name}`, { error: msg });
      return { success: false, output: "", error: msg };
    }
  }
}

export const toolRegistry = new ToolRegistry();
