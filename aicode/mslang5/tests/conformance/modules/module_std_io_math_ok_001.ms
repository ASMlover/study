// @id: module_std_io_math_ok_001
// @spec: docs/spec/modules.md#import-standard-modules
// @expect: ok
// @stdout:
// 3
// 3
// 5
// 3
// 4
// 8
import std.io;
import std.math;

io.println(math.abs(-3));
print math.min(3, 5);
print math.max(3, 5);
print math.floor(3.9);
print math.ceil(3.1);

from std.math import abs as abs_fn;
print abs_fn(-8);
