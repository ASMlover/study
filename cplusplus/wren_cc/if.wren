
if (true) io.write("good") // expect: good
if (false) io.write("bad")

if (true) io.write("good") else io.write("bad") // expect: good
if (false) io.write("bad") else io.write("good") // expect: good

// allow statements for then branch
if (true) { io.write("block") } // expect: block
if (true) if (true) io.write("double") // expect: double

// allow statements for else branch
if (false) nil else { io.write("block") } // expect: block
if (false) nil else if (true) io.write("double") // expect: double

// return the `then` expression if the condition is true
var a = if (true) "good"
io.write(a) // expect: good

// return nil if the condition is false and there is no else
var b = if (false) "bad"
io.write(b) // expect: nil

// return the `else` expression if the condition is false
var c = if (false) "bad" else "good"
io.write(c) // expect: good

// assignment in if condition
if (a = true) io.write(a) // expect: true

// newline after `if`
if
(true) io.write("good") // expect: good

// newline after `else`
if (false) io.write("bad") else
io.write("good") // expect: good

// definition in then branch
if (true) var a = io.write("ok") // expect: ok
if (true) class Foo {}

// definition in else branch
if (false) nil else var a = io.write("ok") // expect: ok
if (true) nil else class Foo {}
