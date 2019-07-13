
if (true) IO.write("good") // expect: good
if (false) IO.write("bad")

if (true) IO.write("good") else IO.write("bad") // expect: good
if (false) IO.write("bad") else IO.write("good") // expect: good

// allow blocks for branches
if (true) { IO.write("block") } // expect: block
if (false) nil else { IO.write("block") } // expect: block

// assignment in if conditIO.
var a = false
if (a = true) IO.write(a) // expect: true

// newline after `if`
if
(true) IO.write("good") // expect: good

// newline after `else`
if (false) IO.write("bad") else
IO.write("good") // expect: good

// only nil and false is falsely
if (0) IO.write(0) // expect: 0
if (nil) IO.write(nil) // no print
if ("") IO.write("empty-string") // expect: empty-string
