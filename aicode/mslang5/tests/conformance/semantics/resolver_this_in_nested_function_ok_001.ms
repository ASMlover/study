// @id: RES-THIS-NESTED-OK-001
// @spec: semantics.class.this
// @spec: semantics.closure.capture
// @expect: ok
// @stdout:
// 9
class Box {
  init(v) {
    this.v = v;
  }

  get() {
    fun capture() {
      return this.v;
    }
    return capture();
  }
}

print Box(9).get();
