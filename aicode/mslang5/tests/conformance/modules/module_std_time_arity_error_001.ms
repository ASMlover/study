// @id: module_std_time_arity_error_001
// @spec: docs/spec/modules.md#import-standard-modules
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4002
// @diag.message_contains: expected 0 arguments but got 1
import std.time;
print time.now_unix_ms(1);