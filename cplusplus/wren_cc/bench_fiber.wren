
var fibers = []
var sum = 0

var start = IO.clock

for (i in 0...100000) {
  fibers.add(new Fiber {
    sum = sum + i
    if (i < 99999) fibers[i + 1].call()
  })
}

fibers[0].call()
IO.print(sum)

IO.print("elapsed: ", IO.clock - start)
