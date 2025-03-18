export class Queue {
  _tasks = [];
  push(task) {
    this._tasks.push(task);
  }

  async execute() {
    while (true) {
      const [task] = this._tasks.splice(0, 1);
      if (!task) {
        break;
      }
      await task();
    }
  }
}
