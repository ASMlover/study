// Test basic generator / coroutine functionality

fun* range_gen(n) {
  var i = 0
  while (i < n) {
    yield i
    i += 1
  }
}

var g = range_gen(3)
print g.next()  // expect: 0
print g.next()  // expect: 1
print g.next()  // expect: 2
print g.next()  // expect: nil

// Test type()
var g2 = range_gen(1)
print type(g2)  // expect: coroutine
