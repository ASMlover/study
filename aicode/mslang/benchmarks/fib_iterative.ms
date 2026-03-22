fun fib_iter(n) {
  if (n < 2) return n
  var a = 0
  var b = 1
  var i = 2
  while (i <= n) {
    var c = a + b
    a = b
    b = c
    i += 1
  }
  return b
}
print fib_iter(32)   // expect: 2178309
