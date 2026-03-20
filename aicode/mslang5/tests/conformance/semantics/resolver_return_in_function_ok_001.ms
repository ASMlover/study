// @id: RES-RETURN-IN-FUNC-OK-001
// @spec: semantics.functions.return
// @expect: ok
// @stdout:
// 42
fun makeValue() {
  var seed = 40
  fun addTwo() {
    return seed + 2
  }
  return addTwo()
}

print makeValue()
