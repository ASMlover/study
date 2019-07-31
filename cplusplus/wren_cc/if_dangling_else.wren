
if (true) if (false) IO.print("bad") else IO.print("good") // expect: good
if (false) if (true) IO.print("bad") else IO.print("bad")
