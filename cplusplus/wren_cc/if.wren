
if (true) IO.print("good") // expect: good
if (false) IO.print("bad")

if (true) IO.print("good") else IO.print("bad") // expect: good
if (false) IO.print("bad") else IO.print("good") // expect: good

// allow blocks for branches
if (true) { IO.print("block") } // expect: block
if (false) nil else { IO.print("block") } // expect: block

// assignment in if conditIO.
var a = false
if (a = true) IO.print(a) // expect: true

// newline after `if`
if
(true) IO.print("good") // expect: good

// newline after `else`
if (false) IO.print("bad") else
IO.print("good") // expect: good

// only nil and false is falsely
if (0) IO.print(0) // expect: 0
if (nil) IO.print(nil) // no print
if ("") IO.print("empty-string") // expect: empty-string
