// Test tab escape
print "--- tab escape ---" // expect: --- tab escape ---
print "hello\tworld" // expect: hello	world

// Test newline escape
print "--- newline escape ---" // expect: --- newline escape ---
print "line1\nline2"
// expect: line1
// expect: line2

// Test backslash escape
print "--- backslash escape ---" // expect: --- backslash escape ---
print "back\\slash" // expect: back\slash

// Test quote escape
print "--- quote escape ---" // expect: --- quote escape ---
print "say \"hi\"" // expect: say "hi"

// Test no escape - regular string
print "--- regular string ---" // expect: --- regular string ---
print "normal" // expect: normal

// Test multiple escapes in one string
print "--- multiple escapes ---" // expect: --- multiple escapes ---
print "a\tb\tc" // expect: a	b	c

// Test escaped backslash before other chars
print "--- escaped backslash before chars ---" // expect: --- escaped backslash before chars ---
print "path\\nfile" // expect: path\nfile
