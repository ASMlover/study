// @id: RUN-UNDEF-PROP-001
// @spec: semantics.class.properties
// @spec: errors.MS4004
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4004
// @diag.message_contains: undefined property
class Box {}
print Box().missing;
