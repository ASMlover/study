// @id: RES-SUPER-OUTSIDE-001
// @spec: errors.MS3003
// @spec: semantics.class.super
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3003
// @diag.message_contains: cannot use 'super' outside of a subclass
print super.nope();
