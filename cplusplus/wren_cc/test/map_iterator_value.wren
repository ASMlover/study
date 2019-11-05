
var a = {1: "one"}

var iterator = a.iterate(nil)
var value = a.iterValue(iterator)

IO.print(value is MapEntry) // expect: true
IO.print(value.key) // expect: 1
IO.print(value.value) // expect: one

a[1] = "updated"
IO.print(value.value) // expect: one
