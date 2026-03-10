// @id: RUN-SUPERCLASS-NON-CLASS-001
// @spec: semantics.class.inheritance
// @spec: errors.MS4003
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4003
// @diag.message_contains: superclass must be a class
var Base = 1;

class Broken < Base {}
