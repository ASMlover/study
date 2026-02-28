import type { Skill } from "../types.js";

export const commitSkill: Skill = {
  name: "commit",
  description: "Generate a git commit for staged changes",
  promptTemplate: `Analyze the current git status and staged changes, then create a git commit.

Steps:
1. Run \`git status\` to see current state
2. Run \`git diff --cached\` to see staged changes (if any). If nothing staged, run \`git diff\` to see unstaged changes
3. Based on the changes, generate a concise, conventional commit message
4. Stage relevant files if needed with \`git add\`
5. Create the commit with the generated message

Use conventional commit format: type(scope): description
Types: feat, fix, refactor, docs, chore, test, style, perf`,
  toolRestrictions: ["shell", "read_file", "list_dir"],
};
