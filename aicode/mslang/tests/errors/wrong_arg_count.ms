// Test: wrong argument count
fun f(a, b) {
  return a + b
}
f(1) // expect runtime error: Expected 2 arguments but got 1.
