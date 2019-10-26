
IO.print("start b")

var B = "b value"
IO.print("b defined ", B)
import "module_cyclic_import_a" for A
IO.print("b imported ", A)

IO.print("end b")
