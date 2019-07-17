
var fib = fn(n) {
  if (n < 2) {
    n
  } else {
    fib.call(n - 1) + fib.call(n - 2)
  }
}

var beg = OS.clock
var i = 0
while (i < 5) {
  IO.write(fib.call(10))
  i = i + 1
}
IO.write("elapsed: " + (OS.clock - beg).toString)
