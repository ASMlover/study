// Fibonacci — recursive implementation
fun fib(n) {
  if (n < 2) return n
  return fib(n - 1) + fib(n - 2)
}

print "Recursive Fibonacci:"
for (var i = 0; i <= 10; i = i + 1) {
  print fib(i)
}

// Fibonacci — iterative implementation
fun fib_iter(n) {
  if (n < 2) return n

  var a = 0
  var b = 1
  for (var i = 2; i <= n; i = i + 1) {
    var temp = a + b
    a = b
    b = temp
  }
  return b
}

print "Iterative Fibonacci:"
for (var i = 0; i <= 20; i = i + 1) {
  print fib_iter(i)
}
