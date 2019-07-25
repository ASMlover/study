
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
    IO.print(_val)
    if (_rhs is Node) {
    _rhs.write
    }
  }
}

var a = new Node
a.set(nil, "a", nil)
var b = new Node
b.set(nil, "b", nil)
var c = new Node
c.set(a, "c", b)
a = nil
b = nil
var d = new Node
d.set(c, "d", nil)
c = nil
d.write

// expect: a
// expect: c
// expect: b
// expect: d
