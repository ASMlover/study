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
