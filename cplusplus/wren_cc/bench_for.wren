
var list = []
for (i in 0...200000) list.add(i)

var start = OS.clock
var sum = 0
for (i in list) {
  sum = sum + i
}
IO.print(sum)
IO.print("use: " + (OS.clock - start).toString)
