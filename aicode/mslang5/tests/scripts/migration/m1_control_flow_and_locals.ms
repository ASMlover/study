var n = 5
var fact = 1
while (n > 1) {
  fact = fact * n
  n = n - 1
}

var marker = ""
for (
  var i = 0
  i < 4
  i = i + 1
) {
  if (i == 1 or i == 3) {
    marker = marker + "T"
  } else {
    marker = marker + "F"
  }
}

{
  var fact = fact - 100
  print fact
}

print fact
print marker