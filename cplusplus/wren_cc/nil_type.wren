
IO.write(nil is Nil) // expect: true
IO.write(nil is Object) // expect: true
IO.write(nil is Bool) // expect: false
IO.write(nil.type == Nil) // expect: true
