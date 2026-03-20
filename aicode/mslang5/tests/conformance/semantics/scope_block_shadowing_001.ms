// @id: SEM-SCOPE-BLOCK-001
// @spec: semantics.scope.lexical
// @spec: semantics.scope.assignment
// @expect: ok
// @stdout:
// block
// block-mut
// global
var name = "global"
{
  var name = "block"
  print name
  name = name + "-mut"
  print name
}
print name
