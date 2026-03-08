// @id: RES-SELF-INHERIT-001
// @spec: errors.MS3004
// @spec: semantics.class.inheritance
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3004
// @diag.message_contains: cannot inherit from itself
class A < A {}
