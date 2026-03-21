// Test: backslash line continuation (\ + newline suppresses ASI)

// --- basic continuation after ASI-trigger tokens ---
print "--- basic continuation ---" // expect: --- basic continuation ---
var x = 10 \
  + 20
print x // expect: 30

var y = "hello" \
  + " world"
print y // expect: hello world

// --- continuation after identifier ---
print "--- identifier continuation ---" // expect: --- identifier continuation ---
var a = 100
var b = a \
  + 50
print b // expect: 150

// --- continuation after closing paren ---
print "--- paren continuation ---" // expect: --- paren continuation ---
fun double(n) {
  return n * 2
}
var r = double(5) \
  + double(3)
print r // expect: 16

// --- continuation after closing bracket ---
print "--- bracket continuation ---" // expect: --- bracket continuation ---
var list = [1, 2, 3]
var elem = list[0] \
  + list[2]
print elem // expect: 4

// --- continuation in multi-line list (closer on own line) ---
print "--- multi-line list ---" // expect: --- multi-line list ---
var list2 = [10, 20, \
  30]
print list2 // expect: [10, 20, 30]

// --- continuation after true/false/nil ---
print "--- bool continuation ---" // expect: --- bool continuation ---
var t = true \
  and true
print t // expect: true

var f = false \
  or true
print f // expect: true

// --- multiple continuations ---
print "--- multiple continuations ---" // expect: --- multiple continuations ---
var z = 1 \
  + 2 \
  + 3 \
  + 4
print z // expect: 10

// --- continuation does not affect next line ---
print "--- next line independent ---" // expect: --- next line independent ---
var p = 5 \
  * 2
var q = 3
print p + q // expect: 13
