// @id: RUN-INVALID-OPERAND-001
// @spec: semantics.value.numeric
// @spec: errors.MS4003
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4003
// @diag.message_contains: operands must be numbers
print 1 - "x"
