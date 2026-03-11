// @id: SEM-CTRL-IF-001
// @spec: semantics.control.if
// @spec: semantics.operators.comparison
// @expect: ok
// @stdout:
// 1
var x = 3;
if (x > 1 and x < 5 and x != 4) {
  print 1;
} else {
  print 0;
}
