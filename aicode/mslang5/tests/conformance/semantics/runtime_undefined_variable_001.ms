// @id: RUN-UNDEF-VAR-001
// @spec: semantics.scope.lexical
// @spec: errors.MS4001
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4001
// @diag.message_contains: undefined variable
print missing_name;
