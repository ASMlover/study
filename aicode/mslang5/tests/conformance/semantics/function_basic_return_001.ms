// @id: SEM-FUNC-RETURN-001
// @spec: semantics.functions.return
// @spec: semantics.functions.arity
// @expect: ok
// @stdout:
// 15
// nil
fun add(a, b) {
  return a + b;
}

fun noValue() {
  return;
}

print add(7, 8);
print noValue();
