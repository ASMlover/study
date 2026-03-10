// @id: RUN-SET-NON-INSTANCE-001
// @spec: semantics.class.fields
// @spec: errors.MS4003
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4003
// @diag.message_contains: only instances have fields
var n = 1;
n.value = 2;
