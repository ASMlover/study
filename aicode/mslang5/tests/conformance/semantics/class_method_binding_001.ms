// @id: SEM-CLASS-METHOD-BIND-001
// @spec: semantics.class.this
// @spec: semantics.class.methods
// @expect: ok
// @stdout:
// 10
// 11
// 11
class Counter {
  init(v) {
    this.v = v;
  }

  inc() {
    this.v = this.v + 1;
    return this.v;
  }
}

var c = Counter(9);
var step = c.inc;
print step();
print step();
print c.v;
