// @id: GRM-ASSIGN-TARGET-001
// @spec: grammar.assignment
// @spec: semantics.expression.assignment_target
// @expect: compile_error
// @diag.phase: parse
// @diag.code: MS4003
// @diag.message_contains: invalid assignment target
(1 + 2) = 3;
