
var list = []
var i = 1
while (i < 4) {
  var j = i + 1
  list.add(new Function { IO.print(j) })
  i = i + 1
}

for (f in list) f.call()
// expect: 2
// expect: 3
// expect: 4
