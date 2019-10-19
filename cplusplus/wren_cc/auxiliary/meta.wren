
class Meta {
  static eval(source_bytes) {
    if (!source_bytes is String) Fiber.abort("source code must be a string")

    var fn = compile(source_bytes)
    if (fn == nil) Fiber.abort("could not compile source code")

    Fiber.new(fn).call()
  }

  foreign static compile(source)
}
