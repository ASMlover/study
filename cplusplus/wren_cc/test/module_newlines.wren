
import

"./module_newlines_module"

import "./module_newlines_module" for

A,

B

// expect: ran module

IO.print(A) // expect: a
IO.print(B) // expect: b
