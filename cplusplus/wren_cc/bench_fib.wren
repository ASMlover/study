
var fib = fn(n) {
  if (n < 2) {
    return n
  } else {
    return fib.call(n - 1) + fib.call(n - 2)
  }
}

var beg = OS.clock
for (i in 1..5) {
  IO.write(fib.call(20))
}
IO.write("elapsed: " + (OS.clock - beg).toString)
