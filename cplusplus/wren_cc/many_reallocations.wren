
var found = []
for (i in 1..1000) {
  var foo = 1337
  for (i in 1..1000) {
    foo = {"a": foo, "b": foo}
  }
  var bar = foo
  for (i in 1..1000) {
    bar = bar["a"]
  }
  found.add(bar)
}

IO.print(found.all{|x| x == 1337 }) // expect: true
IO.print("DONE!") // expect: DONE!
