var s = ""
var i = 0
while (i < 1000) {
  s = s + "x"
  i += 1
}
print strlen(s)   // expect: 1000
