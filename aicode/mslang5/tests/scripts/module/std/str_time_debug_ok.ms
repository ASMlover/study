import std.io
import std.str
import std.time
import std.debug

io.println(str.len("maple"))
io.println(str.substr("maple", 1, 3))
io.println(str.contains("maple", "pl"))

var now = time.now_unix_ms()
io.println(debug.typeof(now))
io.println(now > 0)
io.println(debug.typeof("x"))

class Box {}
var b = Box()
io.println(debug.typeof(b))
io.println(debug.typeof(Box))
io.println(debug.typeof(str.len))

fun foo() {
  return nil
}
io.println(debug.typeof(foo))
