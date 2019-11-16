
import "./module_shared_module_a" for A
import "./module_shared_module_b" for B

// Shared module should only run done
// expect: a
// expect: shared
// expect: a done
// expect: b
// expect: b done

IO.print(A) // expect: a shared
IO.print(B) // expect: b shared
