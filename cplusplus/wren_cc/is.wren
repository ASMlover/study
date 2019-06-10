
io.write(true is Bool) // expect: true
io.write((fn 1) is Function) // expect: true
io.write(123 is Numeric) // expect: true
io.write(nil is Nil) // expect: true
io.write("s" is String) // expect: true

io.write(Numeric is Bool) // expect: false
io.write(nil is Class) // expect: false
io.write(true is Function) // expect: false
io.write((fn 1) is Numeric) // expect: false
io.write("s" is Nil) // expect: false
io.write(123 is String) // expect: false
