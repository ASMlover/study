// Test: stack overflow from deep recursion should produce error, not crash
fun recurse() {
  recurse()
}
recurse()
// expect runtime error: Stack overflow.
