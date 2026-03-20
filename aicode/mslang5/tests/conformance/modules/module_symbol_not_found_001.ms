// @id: MOD-SYMBOL-NOT-FOUND-001
// @spec: modules.from_import.alias
// @spec: errors.MS5002
// @expect: runtime_error
// @diag.phase: module
// @diag.code: MS5002
// @diag.message_contains: has no symbol
from util import not_exported as x
