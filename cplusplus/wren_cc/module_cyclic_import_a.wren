
IO.print("start a")

var A = "a value"
IO.print("a defined ", A)
var B = "module_cyclic_import_b".import_("B")
IO.print("a imported ", B)

IO.print("end a")
