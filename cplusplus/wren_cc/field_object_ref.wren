
class Node {
  this new(lhs, val, rhs) {
    _lhs = lhs
    _val = val
    _rhs = rhs
  }

  write() {
    if (_lhs is Node) {
      _lhs.write()
    }
    IO.print(_val)
    if (_rhs is Node) {
    _rhs.write()
    }
  }
}

var a = Node.new(nil, "a", nil)
var b = Node.new(nil, "b", nil)
var c = Node.new(a, "c", b)
a = nil
b = nil
var d = Node.new(c, "d", nil)
c = nil
d.write()

// expect: a
// expect: c
// expect: b
// expect: d
