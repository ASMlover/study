
var Module = "module_simple_import_module.wren".import_("Module")
// expect: ran module

IO.print(Module) // expect: from module
