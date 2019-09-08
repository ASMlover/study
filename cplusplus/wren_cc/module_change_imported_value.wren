
import "module_change_imported_value_module" for Module
import "module_change_imported_value_module" for Other

IO.print(Module) // expect: before

Other.change
IO.print(Module) // expect: before

Other.show // expect: after
