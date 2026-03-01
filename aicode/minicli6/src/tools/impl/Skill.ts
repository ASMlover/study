import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { SkillManager } from '../../skills/SkillManager.js';

export class SkillTool extends Tool {
  readonly name = 'skill';
  readonly description =
    'Load a specialized skill that provides domain-specific instructions and workflows. When you recognize that a task matches one of the available skills, use this tool to load the full skill instructions. The skill will inject detailed instructions, workflows, and access to bundled resources (scripts, references, templates) into the conversation context.';
  readonly parameters = {
    type: 'object' as const,
    properties: {
      name: {
        type: 'string' as const,
        description: 'The name of the skill from available_skills (e.g., "frontend-design", "skill-creator")',
      },
    },
    required: ['name'],
  };
  permission: 'auto' | 'confirm' | 'deny' = 'auto';

  private skillManager: SkillManager;

  constructor(skillManager: SkillManager) {
    super();
    this.skillManager = skillManager;
  }

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { name } = call.arguments;

    if (!name) {
      return {
        success: false,
        output: '',
        error: 'Missing required parameter: name',
      };
    }

    const skill = this.skillManager.get(name);

    if (!skill) {
      const availableSkills = this.skillManager.list();
      return {
        success: false,
        output: '',
        error: `Skill not found: ${name}. Available skills: ${availableSkills.join(', ') || 'none'}`,
      };
    }

    const skillContext = {
      agent: null,
      workingDirectory: context.workingDirectory,
      config: context.config,
      logger: {
        info: (msg: string) => console.log(`[Skill] ${msg}`),
        error: (msg: string) => console.error(`[Skill Error] ${msg}`),
      },
    };

    const result = await this.skillManager.execute(name, {}, skillContext);

    if (!result.success) {
      return result;
    }

    const output = `<skill_content name="${skill.definition.name}">
${skill.definition.description}

${result.output}
</skill_content>`;

    return {
      success: true,
      output,
      metadata: {
        skillName: skill.definition.name,
        version: skill.definition.version,
      },
    };
  }
}
