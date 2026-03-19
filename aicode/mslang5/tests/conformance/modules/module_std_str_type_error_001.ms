// @id: module_std_str_type_error_001
// @spec: docs/spec/modules.md#import-standard-modules
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4003
// @diag.message_contains: argument 1 must be string
import std.str;
print str.len(123);