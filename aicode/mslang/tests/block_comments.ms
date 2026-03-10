// Block comments
print "--- single line block comment ---"; // expect: --- single line block comment ---
/* single line block comment */
print "after single";        // expect: after single

print "--- multi-line block comment ---"; // expect: --- multi-line block comment ---
/* multi-line
   block
   comment */
print "after multi";         // expect: after multi

print "--- nested block comment ---"; // expect: --- nested block comment ---
/* nested /* inner */ outer */
print "after nested";        // expect: after nested

print "--- deeply nested block comment ---"; // expect: --- deeply nested block comment ---
/* deeply /* nested /* three */ two */ one */
print "after deep";          // expect: after deep

print "--- inline block comment ---"; // expect: --- inline block comment ---
var x = 1 /* inline */ + 2;
print x;                     // expect: 3

// mixed with line comment
print "--- mixed block and line comment ---"; // expect: --- mixed block and line comment ---
/* block */ // line
print "mixed";               // expect: mixed
