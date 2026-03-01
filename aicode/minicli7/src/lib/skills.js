import fs from "node:fs/promises";
import path from "node:path";

export class SkillManager {
  constructor(cwd) {
    this.cwd = cwd;
    this.skillDir = path.join(cwd, ".minicli", "skills");
  }

  async listSkills() {
    try {
      const entries = await fs.readdir(this.skillDir, { withFileTypes: true });
      return entries
        .filter((e) => e.isFile() && e.name.endsWith(".md"))
        .map((e) => e.name.replace(/\.md$/, ""));
    } catch {
      return [];
    }
  }

  async getSkill(name) {
    const file = path.join(this.skillDir, `${name}.md`);
    const content = await fs.readFile(file, "utf8");
    return { name, content, file };
  }

  async buildSkillPrompt(name, input) {
    const skill = await this.getSkill(name);
    return [
      "You must apply the following skill instructions exactly when useful.",
      `Skill name: ${skill.name}`,
      "Skill file content:",
      skill.content,
      "User request tied to this skill:",
      input,
    ].join("\n\n");
  }
}
