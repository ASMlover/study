
var Module = "module_simple_import_module".import_("Module")
// expect: ran module

IO.print(Module) // expect: from module
