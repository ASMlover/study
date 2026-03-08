// @id: MOD-NOT-FOUND-001
// @spec: modules.import.cache
// @spec: errors.MS5001
// @expect: runtime_error
// @diag.phase: module
// @diag.code: MS5001
// @diag.message_contains: module not found
import no_such_module;
