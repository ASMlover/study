
IO.write(123 is Numeric) // expect: true
IO.write(123 is Object) // expect: true
IO.write(123 is String) // expect: false
IO.write(123.type == Numeric) // expect: true
