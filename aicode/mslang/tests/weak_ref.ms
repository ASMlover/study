// Test: Weak references — GC does not trace; sweep nulls out refs to collected objects

// 1. Basic weak_ref creation and get
class Obj {
  init(val) { this.val = val }
}

var obj = Obj(42)
var w = weak_ref(obj)
print w.get().val  // expect: 42
print w.alive()    // expect: true

// 2. Weak ref returns nil after target is unreachable
fun make_weak() {
  var temp = Obj(99)
  return weak_ref(temp)
}

var dead_ref = make_weak()
// temp is now unreachable; force GC by allocating heavily
for (var i = 0; i < 5000; i = i + 1) {
  var junk = [i, i + 1, i + 2, i + 3, i + 4]
}
print dead_ref.get()    // expect: nil
print dead_ref.alive()  // expect: false

// 3. Weak ref to a live object stays valid
var keeper = Obj(7)
var w2 = weak_ref(keeper)
for (var i = 0; i < 1000; i = i + 1) {
  var junk = [i]
}
print w2.get().val  // expect: 7
print w2.alive()    // expect: true

// 4. Multiple weak refs to same object
var shared = Obj(123)
var wa = weak_ref(shared)
var wb = weak_ref(shared)
print wa.get().val  // expect: 123
print wb.get().val  // expect: 123
