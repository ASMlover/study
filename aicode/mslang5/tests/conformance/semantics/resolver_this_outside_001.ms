// @id: RES-THIS-OUTSIDE-001
// @spec: errors.MS3002
// @spec: semantics.class.this
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3002
// @diag.message_contains: cannot use 'this' outside of a class
print this;
