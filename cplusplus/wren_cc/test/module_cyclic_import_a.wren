
IO.print("start a")

var A = "a value"
IO.print("a defined ", A)
import "./module_cyclic_import_b" for B
IO.print("a imported ", B)

IO.print("end a")
