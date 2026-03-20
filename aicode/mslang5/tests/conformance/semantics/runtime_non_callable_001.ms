// @id: RUN-NON-CALLABLE-001
// @spec: semantics.functions.callable
// @spec: errors.MS4005
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4005
// @diag.message_contains: can only call functions and classes
var value = 7
value()
