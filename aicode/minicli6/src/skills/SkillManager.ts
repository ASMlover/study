import { Skill, SkillDefinition, SkillResult, SkillContext } from './Skill.js';
import * as fs from 'fs';
import * as path from 'path';
import { parse } from 'yaml';

export class SkillManager {
  private skills: Map<string, Skill> = new Map();
  private skillDirectories: string[];

  constructor(skillDirectories: string[] = []) {
    this.skillDirectories = skillDirectories;
  }

  async loadAll(): Promise<void> {
    for (const dir of this.skillDirectories) {
      if (!fs.existsSync(dir)) {
        continue;
      }

      const entries = fs.readdirSync(dir, { withFileTypes: true });

      for (const entry of entries) {
        if (entry.isDirectory()) {
          const skillPath = path.join(dir, entry.name, 'skill.yaml');
          if (fs.existsSync(skillPath)) {
            await this.load(skillPath);
          }
        }
      }
    }
  }

  async load(skillPath: string): Promise<Skill | null> {
    try {
      if (!fs.existsSync(skillPath)) {
        return null;
      }

      const content = fs.readFileSync(skillPath, 'utf-8');
      const definition = this.parseSkillYaml(content);

      if (!definition || !definition.name) {
        return null;
      }

      const skill = new Skill(definition);
      this.skills.set(definition.name, skill);

      return skill;
    } catch (error) {
      return null;
    }
  }

  private parseSkillYaml(content: string): SkillDefinition | null {
    try {
      const raw = parse(content);

      if (!raw || !raw.name) {
        return null;
      }

      const definition: SkillDefinition = {
        name: raw.name,
        description: raw.description || '',
        version: raw.version || '1.0.0',
        author: raw.author,
        parameters: this.parseParameters(raw.parameters || []),
        prompt: raw.prompt,
      };

      return definition;
    } catch {
      return null;
    }
  }

  private parseParameters(rawParams: any[]): SkillDefinition['parameters'] {
    return rawParams.map((param) => ({
      name: param.name,
      type: param.type || 'string',
      description: param.description || '',
      required: param.required !== false,
      default: param.default,
    }));
  }

  get(name: string): Skill | undefined {
    return this.skills.get(name);
  }

  getSkillDescriptions(): string {
    const descriptions: string[] = [];

    for (const [name, skill] of this.skills) {
      const params = skill.definition.parameters
        .map((p) => `  - ${p.name}: ${p.description}${p.required ? ' (required)' : ''}`)
        .join('\n');

      descriptions.push(
        `**${name}** (v${skill.definition.version})\n${skill.definition.description}\nParameters:\n${params}`
      );
    }

    return descriptions.join('\n\n');
  }

  async execute(
    name: string,
    params: Record<string, any>,
    context: SkillContext
  ): Promise<SkillResult> {
    const skill = this.skills.get(name);

    if (!skill) {
      return {
        success: false,
        output: '',
        error: `Skill not found: ${name}. Available skills: ${this.list().join(', ')}`,
      };
    }

    return skill.run(params, context);
  }

  list(): string[] {
    return Array.from(this.skills.keys());
  }

  register(definition: SkillDefinition): void {
    const skill = new Skill(definition);
    this.skills.set(definition.name, skill);
  }

  addSkillDirectory(dir: string): void {
    if (!this.skillDirectories.includes(dir)) {
      this.skillDirectories.push(dir);
    }
  }
}
