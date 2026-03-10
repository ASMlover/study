// @id: RES-THIS-FREE-FUNC-001
// @spec: semantics.class.this
// @spec: errors.MS3002
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3002
// @diag.message_contains: cannot use 'this' outside of a class
fun leakThis() {
  return this;
}

print leakThis();
