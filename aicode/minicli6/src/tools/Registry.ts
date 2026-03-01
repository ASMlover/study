import {
  Tool,
  ToolSchema,
  ToolCall,
  ToolResult,
  ToolContext,
} from './Tool.js';

/**
 * Tool registry for managing and executing tools
 * 
 * @example
 * ```typescript
 * const registry = new ToolRegistry();
 * registry.register(new ReadFileTool());
 * registry.register(new WriteFileTool());
 * 
 * // Get schemas for API
 * const schemas = registry.getSchemas();
 * 
 * // Execute a tool
 * const result = await registry.execute('read_file', call, context);
 * ```
 */
export class ToolRegistry {
  private tools: Map<string, Tool> = new Map();

  /**
   * Register a tool
   * @param tool - Tool instance to register
   * @throws Error if tool with same name already exists
   */
  register(tool: Tool): void {
    if (this.tools.has(tool.name)) {
      throw new Error(`Tool "${tool.name}" is already registered`);
    }
    this.tools.set(tool.name, tool);
  }

  /**
   * Register multiple tools at once
   * @param tools - Array of tool instances to register
   */
  registerAll(tools: Tool[]): void {
    for (const tool of tools) {
      this.register(tool);
    }
  }

  /**
   * Get a tool by name
   * @param name - Tool name
   * @returns Tool instance or undefined if not found
   */
  get(name: string): Tool | undefined {
    return this.tools.get(name);
  }

  /**
   * Get all tool schemas for GLM5 API
   * @returns Array of tool schemas
   */
  getSchemas(): ToolSchema[] {
    return Array.from(this.tools.values()).map(tool => tool.toSchema());
  }

  /**
   * Execute a tool by name
   * @param name - Tool name
   * @param call - Tool call request
   * @param context - Execution context
   * @returns Tool execution result
   * @throws Error if tool not found or validation fails
   */
  async execute(
    name: string,
    call: ToolCall,
    context: ToolContext
  ): Promise<ToolResult> {
    const tool = this.tools.get(name);
    
    if (!tool) {
      return {
        success: false,
        output: '',
        error: `Tool "${name}" not found. Available tools: ${this.list().join(', ')}`,
      };
    }

    if (!tool.validate(call.arguments)) {
      return {
        success: false,
        output: '',
        error: `Invalid arguments for tool "${name}". Check required parameters.`,
      };
    }

    try {
      return await tool.execute(call, context);
    } catch (error) {
      return {
        success: false,
        output: '',
        error: error instanceof Error ? error.message : String(error),
      };
    }
  }

  /**
   * Check if a tool exists
   * @param name - Tool name
   * @returns Whether tool exists
   */
  has(name: string): boolean {
    return this.tools.has(name);
  }

  /**
   * Get all registered tool names
   * @returns Array of tool names
   */
  list(): string[] {
    return Array.from(this.tools.keys());
  }

  /**
   * Unregister a tool
   * @param name - Tool name
   * @returns Whether tool was removed
   */
  unregister(name: string): boolean {
    return this.tools.delete(name);
  }

  /**
   * Clear all registered tools
   */
  clear(): void {
    this.tools.clear();
  }

  /**
   * Get number of registered tools
   */
  get size(): number {
    return this.tools.size;
  }
}
