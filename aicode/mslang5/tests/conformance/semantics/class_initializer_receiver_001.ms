// @id: SEM-CLASS-INIT-001
// @spec: semantics.class.initializer
// @expect: ok
// @stdout:
// 5
class Box {
  init(v) {
    this.v = v;
  }

  get() {
    return this.v;
  }
}

var b = Box(5);
print b.get();
