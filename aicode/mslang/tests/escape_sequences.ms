// Test tab escape
print "hello\tworld"; // expect: hello	world

// Test newline escape
print "line1\nline2";
// expect: line1
// expect: line2

// Test backslash escape
print "back\\slash"; // expect: back\slash

// Test quote escape
print "say \"hi\""; // expect: say "hi"

// Test no escape - regular string
print "normal"; // expect: normal

// Test multiple escapes in one string
print "a\tb\tc"; // expect: a	b	c

// Test escaped backslash before other chars
print "path\\nfile"; // expect: path\nfile
