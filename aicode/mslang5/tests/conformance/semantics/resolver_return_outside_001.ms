// @id: RES-RETURN-OUTSIDE-001
// @spec: errors.MS3001
// @spec: semantics.functions.return
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3001
// @diag.message_contains: cannot return from top-level code
return 1
