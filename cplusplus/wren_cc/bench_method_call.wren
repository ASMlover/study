
class Toggle {
  new(startState) {
    _state = startState
  }

  value { return _state }

  activate {
    _state = !_state
    return this
  }
}

class NthToggle is Toggle {
  new(startState, maxCounter) {
    super(startState)
    _countMax = maxCounter
    _count = 0
  }

  activate {
    _count = _count + 1
    if (_count >= _countMax) {
      super.activate
      _count = 0
    }
    return this
  }
}

var start = OS.clock
var n = 100000
var val = true
var toggle = new Toggle(val)

for (i in 0...n) {
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
}
IO.write(toggle.value)

val = true
var ntoggle = new NthToggle(val, 3)

for (i in 0...n) {
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
}
IO.write(ntoggle.value)

IO.write("use: " + (OS.clock - start).toString)
