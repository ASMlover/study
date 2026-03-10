// @id: SEM-CLOSURE-INDEPENDENT-001
// @spec: semantics.closure.capture
// @spec: semantics.scope.lexical
// @expect: ok
// @stdout:
// 1
// 2
// 101
// 3
// 102
fun makeCounter(seed) {
  var n = seed;
  fun inc() {
    n = n + 1;
    return n;
  }
  return inc;
}

var c1 = makeCounter(0);
var c2 = makeCounter(100);
print c1();
print c1();
print c2();
print c1();
print c2();
