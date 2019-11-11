
class Meta {
  static getModuleVariables(module) {
    if (!(module is String)) Fiber.abort("module name must be a string")
    var result = getModuleVars(module)
    if (result != nil) return result

    Fiber.abort("could not find the module named `" + module + "`")
  }

  static eval(source_bytes) {
    if (!source_bytes is String) Fiber.abort("source code must be a string")

    var fiber = compile_(source_bytes, false, false)
    if (fiber == nil) Fiber.abort("could not compile source code")

    fiber.call()
  }

  static compileExpression(source_bytes) {
    if (!(source_bytes is String)) Fiber.abort("source code must be a string")
    return compile_(source_bytes, true, true)
  }

  static compile(source_bytes) {
    if (!(source_bytes is String)) Fiber.abort("source code must be a string")
    return compile_(source_bytes, false, true)
  }

  foreign static compile_(source, is_expression, print_errors)
  foreign static getModuleVars(module)
}
