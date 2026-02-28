import type { Skill } from "../types.js";

export const planSkill: Skill = {
  name: "plan",
  description: "Create an implementation plan for a task",
  promptTemplate: `Create a detailed implementation plan for the user's request.

Steps:
1. Analyze the current codebase structure (use project_tree and read key files)
2. Identify which files need to be created or modified
3. Consider dependencies and ordering
4. Produce a numbered plan with:
   - Each step clearly described
   - Files to create/modify
   - Key decisions and trade-offs
   - Potential risks

Output the plan in a clear markdown format. Do NOT implement anything - just plan.`,
  toolRestrictions: ["read_file", "list_dir", "grep", "glob", "project_tree"],
};
