class Point { init(x, y) { this.x = x; this.y = y; } }
var p = Point(0, 0)
var i = 0
while (i < 1000000) {
  p.x = i
  var _ = p.x
  i += 1
}
print p.x   // expect: 999999
