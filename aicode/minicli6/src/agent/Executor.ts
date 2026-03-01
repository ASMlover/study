import { ToolRegistry } from '../tools/Registry.js';
import { PermissionManager } from '../tools/Permission.js';
import { ToolCall, ToolResult, ToolContext } from '../tools/Tool.js';
import { BashTool, ReadTool, WriteTool, EditTool, GlobTool, GrepTool, TaskTool, TodoWriteTool, SkillTool } from '../tools/impl/index.js';
import { SkillManager } from '../skills/SkillManager.js';
import type { MiniCLIConfig } from '../core/Config.js';
import * as path from 'path';

export interface PermissionDecision {
  allowed: boolean;
  scope: 'once' | 'session' | 'permanent';
}

export class ToolExecutor {
  private registry: ToolRegistry;
  private permissionManager: PermissionManager;
  private context: ToolContext;
  private skillManager: SkillManager;
  
  public onPermissionRequest?: (toolCall: ToolCall, toolName: string) => Promise<PermissionDecision>;

  constructor(config: MiniCLIConfig, workingDirectory: string, sessionId: string) {
    this.registry = new ToolRegistry();
    this.permissionManager = new PermissionManager(config.tools?.permissions || {});
    this.context = {
      workingDirectory,
      sessionId,
      config,
    };
    
    this.skillManager = new SkillManager(this.getSkillDirectories(workingDirectory));
    this.registerTools();
  }
  
  private getSkillDirectories(workingDirectory: string): string[] {
    return [
      path.join(workingDirectory, '.minicli', 'skills'),
      path.join(process.cwd(), 'skills'),
    ];
  }
  
  private async loadSkills(): Promise<void> {
    await this.skillManager.loadAll();
  }
  
  async initialize(): Promise<void> {
    await this.loadSkills();
  }
  
  private registerTools(): void {
    this.registry.registerAll([
      new BashTool(),
      new ReadTool(),
      new WriteTool(),
      new EditTool(),
      new GlobTool(),
      new GrepTool(),
      new TaskTool(),
      new TodoWriteTool(),
      new SkillTool(this.skillManager),
    ]);
  }
  
  getToolSchemas(): any[] {
    return this.registry.getSchemas();
  }
  
  async execute(toolCall: ToolCall): Promise<ToolResult> {
    const tool = this.registry.get(toolCall.name);
    if (!tool) {
      return {
        success: false,
        output: '',
        error: `Unknown tool: ${toolCall.name}`,
      };
    }
    
    const permissionLevel = this.permissionManager.check(toolCall.name);
    
    if (permissionLevel === 'deny') {
      return {
        success: false,
        output: '',
        error: 'Tool execution denied by configuration',
      };
    }
    
    if (permissionLevel === 'confirm') {
      const cachedPermission = this.permissionManager.hasPermission(toolCall.name);
      
      if (cachedPermission === null) {
        if (this.onPermissionRequest) {
          const decision = await this.onPermissionRequest(toolCall, toolCall.name);
          
          if (!decision.allowed) {
            return {
              success: false,
              output: '',
              error: 'Tool execution denied by user',
            };
          }
          
          this.permissionManager.grant(toolCall.name, decision.scope);
        } else {
          return {
            success: false,
            output: '',
            error: 'Permission request handler not configured',
          };
        }
      } else if (!cachedPermission) {
        return {
          success: false,
          output: '',
          error: 'Tool execution denied',
        };
      }
    }
    
    try {
      return await this.registry.execute(toolCall.name, toolCall, this.context);
    } catch (error) {
      return {
        success: false,
        output: '',
        error: error instanceof Error ? error.message : String(error),
      };
    }
  }
}

export type { ToolResult };
