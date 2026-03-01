import { ToolCall } from '../types/index.js';
import { configManager } from '../core/Config.js';

export type PermissionLevel = 'auto' | 'confirm' | 'deny';
export type PermissionScope = 'once' | 'session' | 'permanent';

export interface PermissionDecision {
  allowed: boolean;
  scope: PermissionScope;
}

export interface PermissionCache {
  session: Map<string, boolean>;
  permanent: Set<string>;
}

export class PermissionManager {
  private cache: PermissionCache;
  private configPermissions: Record<string, PermissionLevel>;

  constructor(configPermissions: Record<string, PermissionLevel> = {}) {
    this.cache = {
      session: new Map(),
      permanent: new Set(),
    };
    this.configPermissions = configPermissions;
  }

  private getCacheKey(toolName: string, resource?: string): string {
    return resource ? `${toolName}:${resource}` : toolName;
  }

  check(toolName: string, resource?: string): PermissionLevel {
    const key = this.getCacheKey(toolName, resource);
    
    if (this.configPermissions[key]) {
      return this.configPermissions[key];
    }
    
    if (this.configPermissions[toolName]) {
      return this.configPermissions[toolName];
    }
    
    return 'confirm';
  }

  async requestPermission(toolName: string, toolCall: ToolCall): Promise<PermissionDecision> {
    const level = this.check(toolName);
    
    if (level === 'auto') {
      return { allowed: true, scope: 'once' };
    }
    
    if (level === 'deny') {
      return { allowed: false, scope: 'once' };
    }
    
    const cachedPermission = this.hasPermission(toolName);
    if (cachedPermission === true) {
      return { allowed: true, scope: 'session' };
    }
    if (cachedPermission === false) {
      return { allowed: false, scope: 'session' };
    }
    
    return new Promise((resolve) => {
      this.pendingRequest = { toolName, toolCall, resolve };
    });
  }

  private pendingRequest: {
    toolName: string;
    toolCall: ToolCall;
    resolve: (decision: PermissionDecision) => void;
  } | null = null;

  getPendingRequest(): typeof this.pendingRequest {
    return this.pendingRequest;
  }

  resolvePendingRequest(allowed: boolean, scope: PermissionScope): void {
    if (!this.pendingRequest) {
      return;
    }

    const { toolName, resolve } = this.pendingRequest;
    
    if (allowed) {
      this.grant(toolName, scope);
    } else {
      this.deny(toolName, scope);
    }

    this.pendingRequest = null;
    resolve({ allowed, scope });
  }

  grant(toolName: string, scope: PermissionScope, resource?: string): void {
    const key = this.getCacheKey(toolName, resource);

    switch (scope) {
      case 'once':
        break;
      case 'session':
        this.cache.session.set(key, true);
        break;
      case 'permanent':
        this.cache.permanent.add(key);
        this.configPermissions[key] = 'auto';
        break;
    }
  }

  deny(toolName: string, scope: PermissionScope, resource?: string): void {
    const key = this.getCacheKey(toolName, resource);

    switch (scope) {
      case 'once':
        break;
      case 'session':
        this.cache.session.set(key, false);
        break;
      case 'permanent':
        this.cache.permanent.add(key);
        this.configPermissions[key] = 'deny';
        break;
    }
  }

  hasPermission(toolName: string, resource?: string): boolean | null {
    const key = this.getCacheKey(toolName, resource);

    if (this.cache.session.has(key)) {
      return this.cache.session.get(key) ?? null;
    }

    if (this.cache.permanent.has(key)) {
      return this.configPermissions[key] === 'auto';
    }

    return null;
  }

  clearSession(): void {
    this.cache.session.clear();
    this.pendingRequest = null;
  }

  async savePermanent(): Promise<void> {
    const config = configManager.get();
    const permissions = { ...config.tools.permissions };

    for (const key of this.cache.permanent) {
      if (this.configPermissions[key]) {
        permissions[key] = this.configPermissions[key];
      }
    }

    configManager.set('tools.permissions', permissions);
    await configManager.save();
  }
}

export const permissionManager = new PermissionManager();
