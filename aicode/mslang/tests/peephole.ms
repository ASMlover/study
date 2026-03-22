// Dead code after return — should not crash
fun f(x) {
  if (x > 0) return x
  return -x
  print "unreachable"   // dead
}
print f(3)    // expect: 3
print f(-5)   // expect: 5
