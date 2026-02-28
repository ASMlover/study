import { resolve, relative } from "node:path";

export function withinRoot(projectRoot: string, targetPath: string): boolean {
  const resolved = resolve(projectRoot, targetPath);
  const rel = relative(projectRoot, resolved);
  return !rel.startsWith("..") && !resolve(rel).startsWith(resolve("/"));
}

export function safePath(projectRoot: string, targetPath: string): string {
  const resolved = resolve(projectRoot, targetPath);
  if (!withinRoot(projectRoot, targetPath)) {
    throw new Error(`Path "${targetPath}" escapes project root`);
  }
  return resolved;
}
