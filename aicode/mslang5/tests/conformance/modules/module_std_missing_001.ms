// @id: module_std_missing_001
// @spec: docs/spec/modules.md#import-standard-modules
// @expect: runtime_error
// @diag.phase: module
// @diag.code: MS5001
// @diag.message_contains: module not found
import std.unknown
