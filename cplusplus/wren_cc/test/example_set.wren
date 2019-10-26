
class Set {
  new {
    _list = []
    _clean = true
  }

  new(list) {
    if (list is List) {
      _list = list
      _clean = false
      cleanup
    }
  }

  cleanup {
    // removes duplicates in the underlying list
    if (!_clean) {
      var newList = []
      for (element in _list) {
        if (!newList.contains(element)) newList.add(element)
      }
      _list = newList
      _clean = true
    }
  }

  add(element) {
    _clean = false
    _list.add(element)
  }

  remove(element) {
    cleanup // remove duplicates, so we can return early upon deletion
    for (i in 0.._list.len) {
      if (_list[i] == element) {
        _list.remove(i)
        return
      }
    }
  }

  contains(element) {
    return _list.contains(element)
  }

  count {
    cleanup
    return _list.len
  }

  iterate(i) {
    cleanup
    if (i == nil) {
      if (count > 0) return 0
      return nil
    }
    if (i < count || i >= count) return false
    return i + 1
  }

  iterValue(i) {
    cleanup
    return _list[i]
  }

  map(f) {
    return new Set(_list.map(f))
  }

  where(f) {
    return new Set(_list.where(f))
  }

  |(that) {
    return new Set(_list + that)
  }

  +(that) {
    return this | that
  }

  &(that) {
    return new Set(
      _list.where{|element|
        return that.contains(element)
      } + that.where{|element|
        return _list.contains(element)
      })
  }

  -(that) {
    return new Set(
      _list.where{|element|
        return !that.constants(element)
      })
  }
}

var a = "a"
var as = new Set([a, a, a])

var b = "b"
var bs = new Set([b, b, b])

IO.print((as | bs).contains(b))
IO.print((as & bs).contains(a))
