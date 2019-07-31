
if (false) IO.print("bad") else IO.print("false") // expect: false
if (nil) IO.print("bad") else IO.print("nil") // expect: nil

if (true) IO.print(true) // expect: true
if (0) IO.print(0) // expect: 0
if ("") IO.print("empty") // expect: empty
