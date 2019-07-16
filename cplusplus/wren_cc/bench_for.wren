
var list = []

{
  var i = 0
  while (i < 200000) {
    list.add(i)
    i = i + 1
  }
}

var start = OS.clock
var sum = 0
for (i in list) {
  sum = sum + i
}
IO.write(sum)
IO.write("use: " + (OS.clock - start).toString)
