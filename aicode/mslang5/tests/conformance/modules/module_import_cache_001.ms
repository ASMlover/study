// @id: MOD-IMPORT-CACHE-001
// @spec: modules.import.cache
// @expect: ok
// @stdout:
// init
// 1
from side import x as a;
from side import x as b;
print a;
