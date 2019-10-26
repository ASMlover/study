
var map = {}

for (i in 0...100) {
  map[i] = i

  if (i >= 10) map.remove(i - 10)
}

IO.print(map.len) // expect: 10
