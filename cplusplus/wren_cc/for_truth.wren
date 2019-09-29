
class Iter {
  construct new(value) { _value = value }
  iterate(iterator) { _value }
  iterValue(iterator) { "value" }
}

// false and nil are false
for (n in Iter.new(false)) {
  IO.print("bad")
  break
}

for (n in Iter.new(nil)) {
  IO.print("bad")
  break
}

// everything else is true
for (n in Iter.new(true)) {
  IO.print("true") // expect: true
  break
}

for (n in Iter.new(0)) {
  IO.print(0) // expect: 0
  break
}

for (n in Iter.new("")) {
  IO.print("string") // expect: string
  break
}
