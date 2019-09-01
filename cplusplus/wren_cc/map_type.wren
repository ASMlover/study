
IO.print({} is Map) // expect: true
IO.print({} is Object) // expect: true
IO.print({} is Bool) // expect: false
IO.print({}.type == Map) // expect: true
