import type { Skill } from "../types.js";
import { commitSkill } from "./commit.js";
import { simplifySkill } from "./simplify.js";
import { planSkill } from "./plan.js";

const skills = new Map<string, Skill>();

export function registerSkill(skill: Skill): void {
  skills.set(skill.name, skill);
}

export function getSkill(name: string): Skill | undefined {
  return skills.get(name);
}

export function listSkills(): Skill[] {
  return [...skills.values()];
}

export function skillNames(): string[] {
  return [...skills.keys()];
}

// Register built-in skills
export function initSkills(): void {
  registerSkill(commitSkill);
  registerSkill(simplifySkill);
  registerSkill(planSkill);
}
