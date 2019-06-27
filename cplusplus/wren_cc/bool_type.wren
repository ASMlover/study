
io.write(true is Bool) // expect: true
io.write(true is Object) // expect: true
io.write(true is Numeric) // expect: false
io.write(true.type == Bool) // expect: true
