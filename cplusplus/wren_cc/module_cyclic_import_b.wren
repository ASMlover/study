
IO.print("start b")

var B = "b value"
IO.print("b defined ", B)
var A = "module_cyclic_import_a".import_("A")
IO.print("b imported ", A)

IO.print("end b")
