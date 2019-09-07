
var A = "module_shared_module_a".import_("A")
var B = "module_shared_module_b".import_("B")

// Shared module should only run done
// expect: a
// expect: shared
// expect: a done
// expect: b
// expect: b done

IO.print(A) // expect: a shared
IO.print(B) // expect: b shared
