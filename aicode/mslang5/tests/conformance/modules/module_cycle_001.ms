// @id: MOD-CYCLE-001
// @spec: modules.circular_dependency
// @spec: errors.MS5003
// @expect: runtime_error
// @diag.phase: module
// @diag.code: MS5003
// @diag.message_contains: circular module dependency
import cycle_a
