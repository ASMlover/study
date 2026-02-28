import type { Skill } from "../types.js";

export const simplifySkill: Skill = {
  name: "simplify",
  description: "Review and simplify recently changed code",
  promptTemplate: `Review the recently changed code for opportunities to simplify and improve.

Steps:
1. Run \`git diff\` to see recent changes
2. For each changed file, read the full file for context
3. Look for:
   - Code duplication that can be extracted
   - Over-engineering or unnecessary abstraction
   - Unused imports or variables
   - Opportunities to use simpler patterns
   - Clarity improvements
4. Make the simplifying changes directly
5. Summarize what was simplified and why`,
  toolRestrictions: ["shell", "read_file", "write_file", "grep", "list_dir"],
};
