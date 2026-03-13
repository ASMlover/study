fun make_accumulator(base) {
  var current = base;
  fun add(step) {
    current = current + step;
    return current;
  }
  return add;
}

fun fib(n) {
  if (n < 2) {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

var acc = make_accumulator(10);
print acc(1);
print acc(4);
print fib(6);