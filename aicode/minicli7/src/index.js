import path from "node:path";
import { fileURLToPath } from "node:url";
import { loadConfig } from "./lib/config.js";
import { UI } from "./lib/ui.js";
import { Agent } from "./lib/agent.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function main() {
  const cwd = process.cwd();
  const ui = new UI();
  const config = await loadConfig(cwd);

  ui.printBanner(config);

  const agent = new Agent({
    cwd,
    config,
    ui,
    appDir: __dirname,
  });

  await agent.runInteractive();
}

main().catch((err) => {
  console.error("Fatal error:", err);
  process.exitCode = 1;
});
