
IO.print(true is Bool) // expect: true
IO.print(true is Object) // expect: true
IO.print(true is Numeric) // expect: false
IO.print(true.type == Bool) // expect: true
