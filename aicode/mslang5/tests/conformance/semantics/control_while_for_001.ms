// @id: SEM-CTRL-LOOP-001
// @spec: semantics.control.while
// @spec: semantics.control.for
// @spec: semantics.scope.lexical
// @expect: ok
// @stdout:
// 15
var total = 0;
for (var i = 0; i < 5; i = i + 1) {
  total = total + i;
}
while (total < 15) {
  total = total + 1;
}
print total;
