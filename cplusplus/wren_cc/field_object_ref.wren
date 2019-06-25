
class Node {
  set(lhs, val, rhs) {
    _lhs = lhs
    _val = val
    _rhs = rhs
  }

  write {
    if (_lhs is Node) {
      _lhs.write
    }
    io.write(_val)
    if (_rhs is Node) {
    _rhs.write
    }
  }
}

var a = Node.new
a.set(nil, "a", nil)
var b = Node.new
b.set(nil, "b", nil)
var c = Node.new
c.set(a, "c", b)
a = nil
b = nil
var d = Node.new
d.set(c, "d", nil)
c = nil
d.write

// expect: a
// expect: c
// expect: b
// expect: d
