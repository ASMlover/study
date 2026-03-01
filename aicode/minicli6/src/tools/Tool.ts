/**
 * Tool parameter definition for JSON Schema compatible parameter descriptions
 */
export interface ToolParameter {
  /** Parameter type */
  type: 'string' | 'number' | 'boolean' | 'object' | 'array';
  /** Parameter description */
  description: string;
  /** Enum values if applicable */
  enum?: (string | number)[];
  /** Default value */
  default?: any;
  /** Nested properties for object type */
  properties?: Record<string, ToolParameter>;
  /** Required properties for object type */
  required?: string[];
  /** Item schema for array type */
  items?: ToolParameter;
}

/**
 * Tool schema definition compatible with GLM5 function calling
 */
export interface ToolSchema {
  /** Tool name (unique identifier) */
  name: string;
  /** Tool description for AI understanding */
  description: string;
  /** Parameters schema */
  parameters: {
    type: 'object';
    properties: Record<string, ToolParameter>;
    required?: string[];
  };
}

/**
 * Tool call request from AI model
 */
export interface ToolCall {
  /** Unique call identifier */
  id: string;
  /** Tool name to invoke */
  name: string;
  /** Arguments passed to the tool */
  arguments: Record<string, any>;
}

/**
 * Tool execution result
 */
export interface ToolResult {
  /** Whether execution succeeded */
  success: boolean;
  /** Output content (shown to user/AI) */
  output: string;
  /** Error message if failed */
  error?: string;
  /** Additional metadata */
  metadata?: Record<string, any>;
}

/**
 * Execution context provided to tools
 */
export interface ToolContext {
  /** Current working directory */
  workingDirectory: string;
  /** Current session identifier */
  sessionId: string;
  /** Configuration object */
  config: any;
}

/**
 * Abstract base class for all tools
 * 
 * @example
 * ```typescript
 * class ReadFileTool extends Tool {
 *   readonly name = 'read_file';
 *   readonly description = 'Read file contents';
 *   readonly parameters = {
 *     type: 'object' as const,
 *     properties: {
 *       path: { type: 'string', description: 'File path' }
 *     },
 *     required: ['path']
 *   };
 * 
 *   async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
 *     // Implementation
 *   }
 * }
 * ```
 */
export abstract class Tool {
  /** Unique tool name identifier */
  abstract readonly name: string;
  
  /** Human-readable description for AI understanding */
  abstract readonly description: string;
  
  /** JSON Schema compatible parameters definition */
  abstract readonly parameters: ToolSchema['parameters'];
  
  /** Permission level: auto (no confirm), confirm (ask user), deny (always reject) */
  permission: 'auto' | 'confirm' | 'deny' = 'confirm';

  /**
   * Convert tool definition to GLM5 compatible schema
   */
  toSchema(): ToolSchema {
    return {
      name: this.name,
      description: this.description,
      parameters: this.parameters,
    };
  }

  /**
   * Execute the tool with given call and context
   * @param call - Tool call request
   * @param context - Execution context
   * @returns Execution result
   */
  abstract execute(call: ToolCall, context: ToolContext): Promise<ToolResult>;

  /**
   * Validate arguments against parameter schema
   * @param args - Arguments to validate
   * @returns Whether arguments are valid
   */
  validate(args: Record<string, any>): boolean {
    const required = this.parameters.required || [];
    
    for (const key of required) {
      if (!(key in args) || args[key] === undefined || args[key] === null) {
        return false;
      }
    }
    
    for (const key of Object.keys(args)) {
      if (!this.parameters.properties[key]) {
        continue;
      }
      
      const param = this.parameters.properties[key];
      const value = args[key];
      
      if (!this.validateType(value, param)) {
        return false;
      }
    }
    
    return true;
  }

  /**
   * Validate a value against parameter type definition
   */
  private validateType(value: any, param: ToolParameter): boolean {
    if (value === undefined || value === null) {
      return true;
    }

    switch (param.type) {
      case 'string':
        return typeof value === 'string';
      case 'number':
        return typeof value === 'number' && !isNaN(value);
      case 'boolean':
        return typeof value === 'boolean';
      case 'object':
        if (typeof value !== 'object' || Array.isArray(value)) {
          return false;
        }
        if (param.properties) {
          const nestedRequired = param.required || [];
          for (const key of nestedRequired) {
            if (!(key in value)) {
              return false;
            }
          }
        }
        return true;
      case 'array':
        if (!Array.isArray(value)) {
          return false;
        }
        if (param.items) {
          return value.every(item => this.validateType(item, param.items!));
        }
        return true;
      default:
        return false;
    }
  }
}
