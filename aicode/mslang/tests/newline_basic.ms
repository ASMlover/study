// Test: basic newline-as-statement-terminator (ASI)

// --- simple statements without semicolons ---
print "--- no semicolons ---" // expect: --- no semicolons ---
var x = 10
print x // expect: 10
var y = 20
print y // expect: 20
print x + y // expect: 30

// --- semicolons still work (backward compat) ---
print "--- explicit semicolons ---" // expect: --- explicit semicolons ---
var a = 1; var b = 2
print a // expect: 1
print b // expect: 2

// --- multiple statements on one line with semicolons ---
print "--- same-line split ---" // expect: --- same-line split ---
var c = 3; var d = 4; print c + d // expect: 7

// --- blank lines between statements ---
print "--- blank lines ---" // expect: --- blank lines ---

var e = 100

print e // expect: 100

var f = 200

print f // expect: 200

// --- mixed: some with semicolons, some without ---
print "--- mixed ---" // expect: --- mixed ---
var g = 5
var h = 6
print g + h // expect: 11
print g * h // expect: 30

// --- expressions as statements ---
print "--- expressions ---" // expect: --- expressions ---
var i = 0
i = 42
print i // expect: 42

// --- string values ---
print "--- strings ---" // expect: --- strings ---
var s = "hello"
print s // expect: hello
var s2 = "world"
print s + " " + s2 // expect: hello world

// --- boolean and nil ---
print "--- bool and nil ---" // expect: --- bool and nil ---
print true // expect: true
print false // expect: false
print nil // expect: nil

// --- after closers: ), ], } ---
print "--- closers ---" // expect: --- closers ---
var list = [1, 2, 3]
print list // expect: [1, 2, 3]
print list[0] // expect: 1

fun add(a, b) {
  return a + b
}
print add(10, 20) // expect: 30
