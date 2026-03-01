import { listTasks, isBlocked, getTask } from "./manager.js";
import { getTheme } from "../tui/theme.js";
import type { Task } from "../types.js";

export function getNextAvailableTask(owner?: string): Task | undefined {
  const all = listTasks();

  return all.find(t => {
    if (t.status !== "pending") return false;
    if (isBlocked(t)) return false;
    if (owner && t.owner && t.owner !== owner) return false;
    return true;
  });
}

export function getTasksByStatus(status: string): Task[] {
  return listTasks().filter(t => t.status === status);
}

export function formatTaskList(tasks?: Task[]): string {
  const list = tasks ?? listTasks();
  const t = getTheme();

  if (list.length === 0) return `  ${t.dim}No tasks.${t.reset}`;

  const STATUS_STYLE: Record<string, { icon: string; color: string }> = {
    pending:     { icon: "○", color: t.muted },
    in_progress: { icon: "◑", color: t.warning },
    completed:   { icon: "●", color: t.success },
  };

  return list
    .map(task => {
      const st = STATUS_STYLE[task.status] ?? { icon: "?", color: t.dim };
      const blocked = isBlocked(task) ? ` ${t.error}[blocked]${t.reset}` : "";
      const owner = task.owner ? ` ${t.dim}@${task.owner}${t.reset}` : "";
      return `  ${st.color}${st.icon}${t.reset} ${t.dim}#${task.id}${t.reset} ${task.subject}${owner}${blocked}`;
    })
    .join("\n");
}
