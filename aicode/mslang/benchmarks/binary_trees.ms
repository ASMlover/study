class Tree {
  init(depth) {
    if (depth == 0) {
      this.left = nil
      this.right = nil
    } else {
      this.left = Tree(depth - 1)
      this.right = Tree(depth - 1)
    }
  }
  check() {
    if (this.left == nil) return 1
    return 1 + this.left.check() + this.right.check()
  }
}

var max_depth = 10
var t = Tree(max_depth)
print t.check()   // expect: 2047
