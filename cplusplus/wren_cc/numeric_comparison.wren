
io.write(1 < 2) // expect: true
io.write(2 < 2) // expect: false
io.write(2 < 1) // expect: false

io.write(1 <= 2) // expect: true
io.write(2 <= 2) // expect: true
io.write(2 <= 1) // expect: false

io.write(1 > 2) // expect: false
io.write(2 > 2) // expect: false
io.write(2 > 1) // expect: true

io.write(1 >= 2) // expect: false
io.write(2 >= 2) // expect: true
io.write(2 >= 1) // expect: true
