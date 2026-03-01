export class TaskManager {
  constructor() {
    this.tasks = [];
    this.nextId = 1;
  }

  addTask({ title, deps = [], goal = "" }) {
    const task = {
      id: this.nextId++,
      title,
      deps,
      goal,
      status: "pending",
      createdAt: new Date().toISOString(),
    };
    this.tasks.push(task);
    return task;
  }

  list() {
    return [...this.tasks];
  }

  getReadyTasks() {
    return this.tasks.filter((task) => {
      if (task.status !== "pending") {
        return false;
      }
      return task.deps.every((depId) => {
        const dep = this.tasks.find((t) => t.id === depId);
        return dep && dep.status === "done";
      });
    });
  }

  updateStatus(id, status) {
    const task = this.tasks.find((t) => t.id === id);
    if (!task) {
      return null;
    }
    task.status = status;
    if (status === "done" || status === "failed") {
      task.finishedAt = new Date().toISOString();
    }
    return task;
  }
}
