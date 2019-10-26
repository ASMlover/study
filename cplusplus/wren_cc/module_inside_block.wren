
var Module = "outer"

if (true) {
  import "module_inside_block_module" for Module
  // expect: ran module

  IO.print(Module) // expect: from module
}

IO.print(Module) // expect: outer
