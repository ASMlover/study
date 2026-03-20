// @id: RES-RETURN-OUTSIDE-BLOCK-001
// @spec: semantics.functions.return
// @spec: errors.MS3001
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3001
// @diag.message_contains: cannot return from top-level code
{
  return 7
}
