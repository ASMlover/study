import { listTasks, isBlocked, getTask } from "./manager.js";
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
  if (list.length === 0) return "No tasks.";

  const STATUS_ICON: Record<string, string> = {
    pending: "○",
    in_progress: "◑",
    completed: "●",
  };

  return list
    .map(t => {
      const icon = STATUS_ICON[t.status] ?? "?";
      const blocked = isBlocked(t) ? " [BLOCKED]" : "";
      const owner = t.owner ? ` @${t.owner}` : "";
      return `${icon} #${t.id} [${t.status}] ${t.subject}${owner}${blocked}`;
    })
    .join("\n");
}
