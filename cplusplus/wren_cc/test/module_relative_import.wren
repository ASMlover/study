
import "./module_sub/module"
import "./module_sub/././dir/module"

// expect: module_sub/module
// expect: module_sub/module_2
// expect: module_sub/dir/module
// expect: module_sub/dir/module_2
// expect: module_sub/module_3
// expect: module_relative_import_module_3
