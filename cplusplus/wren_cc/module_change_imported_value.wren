
var Module = "module_change_imported_value_module.wren".import_("Module")
var Other = "module_change_imported_value_module.wren".import_("Other")

IO.print(Module) // expect: before

Other.change
IO.print(Module) // expect: before

Other.show // expect: after
