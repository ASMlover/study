// generated automatically from builtin\meta.wren, do not edit

static const str_t kLibSource =
"\n"
"class Meta {\n"
"  static eval(source_bytes) {\n"
"    if (!source_bytes is String) Fiber.abort(\"source code must be a string\")\n"
"\n"
"    var fn = compile(source_bytes)\n"
"    if (fn == nil) Fiber.abort(\"could not compile source code\")\n"
"\n"
"    Fiber.new(fn).call()\n"
"  }\n"
"\n"
"  foreign static compile(source)\n"
"}\n";
