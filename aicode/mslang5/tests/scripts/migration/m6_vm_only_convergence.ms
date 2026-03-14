fun make_adder(offset) {
  fun add(v) {
    return offset + v;
  }
  return add;
}

class BaseAcc {
  init(seed) {
    this.seed = seed;
  }

  total(v) {
    return this.seed + v;
  }
}

class Acc < BaseAcc {
  init(seed) {
    super.init(seed);
  }

  total(v) {
    return super.total(v) + 20;
  }
}

var add = make_adder(7);
var acc = Acc(10);
print add(5);
print acc.total(4);