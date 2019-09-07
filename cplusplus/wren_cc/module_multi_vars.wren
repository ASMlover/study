
var Module1 = "module_multi_vars_module".import_("Module1")
var Module2 = "module_multi_vars_module".import_("Module2")
var Module3 = "module_multi_vars_module".import_("Module3")
var Module4 = "module_multi_vars_module".import_("Module4")
var Module5 = "module_multi_vars_module".import_("Module5")

IO.print(Module1) // expect: from module 1
IO.print(Module2) // expect: from module 2
IO.print(Module3) // expect: from module 3
IO.print(Module4) // expect: from module 4
IO.print(Module5) // expect: from module 5
