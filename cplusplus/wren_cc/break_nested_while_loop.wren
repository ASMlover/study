
var i = 0
while (true) {
  IO.print("outer " + i.toString)
  if (i > 1) break

  var j = 0
  while (true) {
    IO.print("inner " + j.toString)
    if (j > 1) break

    j = j + 1
  }
  i = i + 1
}

// expect: outer 0
// expect: inner 0
// expect: inner 1
// expect: inner 2
// expect: outer 1
// expect: inner 0
// expect: inner 1
// expect: inner 2
// expect: outer 2
