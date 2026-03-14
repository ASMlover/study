// @id: GRM-ASSIGN-TARGET-001
// @spec: grammar.assignment
// @spec: semantics.expression.assignment_target
// @expect: compile_error
// @diag.phase: parse
// @diag.code: MS2001
// @diag.message_contains: expected ';' after expression
(1 + 2) = 3;
