
var list = []

var start = IO.clock
for (i in 0...200000) list.add(i)
var sum = 0
for (i in list) sum = sum + i
IO.print(sum)
IO.print("use: ", IO.clock - start)
