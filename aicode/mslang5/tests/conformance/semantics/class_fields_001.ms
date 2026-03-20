// @id: SEM-CLASS-FIELD-001
// @spec: semantics.class.fields
// @spec: semantics.class.this
// @expect: ok
// @stdout:
// 7
// 9
class Bag {
  init(v) {
    this.v = v
  }

  get() {
    return this.v
  }
}

var b = Bag(7)
print b.get()
b.v = 9
print b.v
