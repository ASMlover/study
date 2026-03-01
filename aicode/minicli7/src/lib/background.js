import { spawn } from "node:child_process";

export class BackgroundManager {
  constructor(cwd) {
    this.cwd = cwd;
    this.jobs = [];
    this.nextId = 1;
  }

  start(command) {
    const id = this.nextId++;
    const job = {
      id,
      command,
      status: "running",
      output: "",
      createdAt: new Date().toISOString(),
    };

    const child = spawn(command, {
      cwd: this.cwd,
      shell: true,
      windowsHide: true,
    });

    job.pid = child.pid;

    child.stdout.on("data", (buf) => {
      job.output += buf.toString("utf8");
      if (job.output.length > 16000) {
        job.output = job.output.slice(-16000);
      }
    });

    child.stderr.on("data", (buf) => {
      job.output += buf.toString("utf8");
      if (job.output.length > 16000) {
        job.output = job.output.slice(-16000);
      }
    });

    child.on("exit", (code) => {
      job.status = code === 0 ? "done" : "failed";
      job.exitCode = code;
      job.finishedAt = new Date().toISOString();
    });

    this.jobs.push(job);
    return job;
  }

  list() {
    return [...this.jobs];
  }
}
