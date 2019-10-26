
var a = []
for (i in 0..200) {
  a.add(i)
}

for (i in 0..195) {
  a.remove(-1)
}

IO.print(a) // expect: [0, 1, 2, 3, 4]
