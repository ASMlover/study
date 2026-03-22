var lst = []
var i = 0
while (i < 10000) {
  lst = lst + [i]
  i += 1
}
var sum = 0
for (var x in lst) {
  sum = sum + x
}
print sum   // expect: 49995000
