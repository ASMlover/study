
if (true) io.write("good") // expect: good
if (false) io.write("bad")

if (true) io.write("good") else io.write("bad") // expect: good
if (false) io.write("bad") else io.write("good") // expect: good

// allow blocks for branches
if (true) { io.write("block") } // expect: block
if (false) nil else { io.write("block") } // expect: block

// assignment in if condition
var a = false
if (a = true) io.write(a) // expect: true

// newline after `if`
if
(true) io.write("good") // expect: good

// newline after `else`
if (false) io.write("bad") else
io.write("good") // expect: good

// only nil and false is falsely
if (0) io.write(0) // expect: 0
if (nil) io.write(nil) // no print
if ("") io.write("empty-string") // expect: empty-string
