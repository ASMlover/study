// @id: RUN-ARITY-001
// @spec: semantics.functions.arity
// @spec: errors.MS4002
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4002
// @diag.message_contains: expected 2 arguments but got 1
fun add(a, b) {
  return a + b
}

print add(1)
