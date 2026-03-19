class Box {
  init(v) {
    this.v = v
    return "ignored"
  }

  get() {
    return this.v
  }
}

var b = Box(5)
print b.get()

