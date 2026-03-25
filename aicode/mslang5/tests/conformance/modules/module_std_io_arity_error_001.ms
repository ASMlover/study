// @id: module_std_io_arity_error_001
// @spec: docs/spec/modules.md#import-standard-modules
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4002
// @diag.message_contains: expected 1 arguments but got 0
import std.io
io.println()
