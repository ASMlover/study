
io.write(123 is Numeric) // expect: true
io.write(123 is Object) // expect: true
io.write(123 is String) // expect: false
io.write(123.type == Numeric) // expect: true
