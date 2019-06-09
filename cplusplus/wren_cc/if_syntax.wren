
if (true) io.write("good") // expect: good
if (false) io.write("bad")

if (true) io.write("good") else io.write("bad") // expect: good
if (false) io.write("bad") else io.write("good") // expect: good

io.write(if (true) "good") // expect: good
io.write(if (false) "bad") // expect: nil
io.write(if (false) "bad" else "good") // expect: good
