
// false and nil are false
while (false) {
  IO.print("bad")
  break
}

while (nil) {
  IO.print("bad")
  break
}

// everything else is true
while (true) {
  IO.print(true) // expect: true
  break
}

while (0) {
  IO.print(0) // expect: 0
  break
}

while ("") {
  IO.print("string") // expect: string
  break
}
