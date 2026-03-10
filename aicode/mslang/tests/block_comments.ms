// Block comments
/* single line block comment */
print "after single";        // expect: after single

/* multi-line
   block
   comment */
print "after multi";         // expect: after multi

/* nested /* inner */ outer */
print "after nested";        // expect: after nested

/* deeply /* nested /* three */ two */ one */
print "after deep";          // expect: after deep

var x = 1 /* inline */ + 2;
print x;                     // expect: 3

// mixed with line comment
/* block */ // line
print "mixed";               // expect: mixed
