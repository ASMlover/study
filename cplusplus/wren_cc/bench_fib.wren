
var fib = fn(n) {
  if (n < 2) return n
  return fib.call(n - 1) + fib.call(n - 2)
}

var beg = IO.clock
for (i in 1..5) {
  IO.print(fib.call(20))
}
IO.print("elapsed: " + (IO.clock - beg).toString)
