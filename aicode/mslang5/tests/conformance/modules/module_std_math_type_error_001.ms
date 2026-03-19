// @id: module_std_math_type_error_001
// @spec: docs/spec/modules.md#import-standard-modules
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4003
// @diag.message_contains: argument 1 must be number
import std.math;
print math.abs("x");
