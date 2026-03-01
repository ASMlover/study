export interface SkillParameter {
  name: string;
  type: 'string' | 'number' | 'boolean';
  description: string;
  required: boolean;
  default?: any;
}

export interface SkillDefinition {
  name: string;
  description: string;
  version: string;
  author?: string;
  parameters: SkillParameter[];
  prompt?: string;
}

export interface SkillContext {
  agent: any;
  workingDirectory: string;
  config: any;
  logger: {
    info: (msg: string) => void;
    error: (msg: string) => void;
  };
}

export interface SkillResult {
  success: boolean;
  output: string;
  error?: string;
}

export class Skill {
  definition: SkillDefinition;

  constructor(definition: SkillDefinition) {
    this.definition = definition;
  }

  validateParams(params: Record<string, any>): { valid: boolean; errors: string[] } {
    const errors: string[] = [];

    for (const param of this.definition.parameters) {
      if (param.required && (params[param.name] === undefined || params[param.name] === null)) {
        errors.push(`Missing required parameter: ${param.name}`);
        continue;
      }

      const value = params[param.name] !== undefined ? params[param.name] : param.default;

      if (value !== undefined && value !== null) {
        const expectedType = param.type;
        const actualType = typeof value;

        if (expectedType === 'number' && actualType !== 'number') {
          errors.push(`Parameter ${param.name} must be a number, got ${actualType}`);
        } else if (expectedType === 'string' && actualType !== 'string') {
          errors.push(`Parameter ${param.name} must be a string, got ${actualType}`);
        } else if (expectedType === 'boolean' && actualType !== 'boolean') {
          errors.push(`Parameter ${param.name} must be a boolean, got ${actualType}`);
        }
      }
    }

    return {
      valid: errors.length === 0,
      errors,
    };
  }

  renderPrompt(params: Record<string, any>): string {
    if (!this.definition.prompt) {
      return '';
    }

    let prompt = this.definition.prompt;
    const resolvedParams = this.resolveParams(params);

    for (const [key, value] of Object.entries(resolvedParams)) {
      const placeholder = `{{${key}}}`;
      prompt = prompt.replace(new RegExp(placeholder, 'g'), String(value));
    }

    return prompt;
  }

  private resolveParams(params: Record<string, any>): Record<string, any> {
    const resolved: Record<string, any> = {};

    for (const param of this.definition.parameters) {
      if (params[param.name] !== undefined) {
        resolved[param.name] = params[param.name];
      } else if (param.default !== undefined) {
        resolved[param.name] = param.default;
      }
    }

    return resolved;
  }

  async run(params: Record<string, any>, context: SkillContext): Promise<SkillResult> {
    const validation = this.validateParams(params);

    if (!validation.valid) {
      return {
        success: false,
        output: '',
        error: `Parameter validation failed: ${validation.errors.join(', ')}`,
      };
    }

    try {
      const prompt = this.renderPrompt(params);

      return {
        success: true,
        output: prompt,
      };
    } catch (error) {
      return {
        success: false,
        output: '',
        error: error instanceof Error ? error.message : String(error),
      };
    }
  }
}
