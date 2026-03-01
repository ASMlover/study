export class SessionStore {
  constructor() {
    this.messages = [];
    this.todos = [];
    this.nextTodoId = 1;
  }

  addMessage(message) {
    this.messages.push(message);
  }

  addTodo(text) {
    const todo = {
      id: this.nextTodoId++,
      text,
      done: false,
      createdAt: new Date().toISOString(),
    };
    this.todos.push(todo);
    return todo;
  }

  markTodoDone(id) {
    const todo = this.todos.find((t) => t.id === id);
    if (!todo) {
      return null;
    }
    todo.done = true;
    todo.doneAt = new Date().toISOString();
    return todo;
  }

  listTodos() {
    return [...this.todos];
  }
}
