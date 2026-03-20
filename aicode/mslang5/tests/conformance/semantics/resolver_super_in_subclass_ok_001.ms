// @id: RES-SUPER-SUBCLASS-OK-001
// @spec: semantics.class.super
// @spec: semantics.class.inheritance
// @expect: ok
// @stdout:
// base-mid-leaf
class Base {
  ping() {
    return "base"
  }
}

class Mid < Base {
  ping() {
    return super.ping() + "-mid"
  }
}

class Leaf < Mid {
  ping() {
    return super.ping() + "-leaf"
  }
}

print Leaf().ping()
