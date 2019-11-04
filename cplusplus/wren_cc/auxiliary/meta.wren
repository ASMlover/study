
class Meta {
  static getModuleVariables(module) {
    if (!(module is String)) Fiber.abort("module name must be a string")
    var result = getModuleVars(module)
    if (result != nil) return result

    Fiber.abort("could not find the module named `" + module + "`")
  }

  static eval(source_bytes) {
    if (!source_bytes is String) Fiber.abort("source code must be a string")

    var fn = compile(source_bytes, false, false)
    if (fn == nil) Fiber.abort("could not compile source code")

    Fiber.new(fn).call()
  }

  static compileExpression(source_bytes) {
    if (!(source_bytes is String)) Fiber.abort("source code must be a string")
    return compile(source_bytes, true, true)
  }

  foreign static compile(source, is_expression, print_errors)
  foreign static getModuleVars(module)
}
