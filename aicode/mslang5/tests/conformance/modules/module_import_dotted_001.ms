// @id: MOD-IMPORT-DOTTED-001
// @spec: modules.import.path
// @spec: modules.from_import.alias
// @expect: ok
// @stdout:
// 42
import pkg.math
from pkg.math import answer as v
print v
