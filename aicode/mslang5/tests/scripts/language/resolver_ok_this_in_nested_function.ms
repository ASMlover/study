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
