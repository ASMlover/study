class Res {
  init(name) { this.name = name; }
  __finalize() { print "fin:" + this.name; }
}
fun make() { var r = Res("temp"); }
make();
for (var i = 0; i < 5000; i = i + 1) {  // expect: fin:temp
  var junk = [i, i + 1, i + 2, i + 3, i + 4];
}
print "1";               // expect: 1
class Other {
  init(id) { this.id = id; }
  __finalize() { print "other"; }
}
print "2";               // expect: 2
var t = Other(999);
print "3";               // expect: 3
for (var i = 0; i < 1000; i = i + 1) {
  var junk = [i];
}
print "4";               // expect: 4
