
IO.print("s" is String) // expect: true
IO.print("s" is Object) // expect: true
IO.print("s" is Numeric) // expect: false
IO.print("s".type == String) // expect: true
