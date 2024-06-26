
class Fib {
  static get(n) {
    if (n < 2) return n
    return get(n - 1) + get(n - 2)
  }
}

var beg = IO.clock
for (i in 1..5) {
  IO.print(Fib.get(20))
}
IO.print("elapsed: ", IO.clock - beg)
