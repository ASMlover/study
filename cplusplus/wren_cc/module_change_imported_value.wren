
import "module_change_imported_value_module" for Module, Other

IO.print(Module) // expect: before

Other.change
IO.print(Module) // expect: before

Other.show // expect: after
