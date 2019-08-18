
class Iter {
  new(value) { _value = value }
  iterate(iterator) { _value }
  iterValue(iterator) { "value" }
}

// false and nil are false
for (n in new Iter(false)) {
  IO.print("bad")
  break
}

for (n in new Iter(nil)) {
  IO.print("bad")
  break
}

// everything else is true
for (n in new Iter(true)) {
  IO.print("true") // expect: true
  break
}

for (n in new Iter(0)) {
  IO.print(0) // expect: 0
  break
}

for (n in new Iter("")) {
  IO.print("string") // expect: string
  break
}
