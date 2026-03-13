// Test range() function (15.3)

// range(stop) — [0, stop)
print range(5);           // expect: [0, 1, 2, 3, 4]
print range(0);           // expect: []
print range(1);           // expect: [0]

// range(start, stop) — [start, stop)
print range(2, 6);        // expect: [2, 3, 4, 5]
print range(-2, 3);       // expect: [-2, -1, 0, 1, 2]
print range(5, 5);        // expect: []

// range(start, stop, step)
print range(0, 10, 2);    // expect: [0, 2, 4, 6, 8]
print range(1, 10, 3);    // expect: [1, 4, 7]

// Negative step (count down)
print range(5, 0, -1);    // expect: [5, 4, 3, 2, 1]
print range(10, 0, -3);   // expect: [10, 7, 4, 1]

// range returns a list — indexable and has len()
var r = range(3);
print r[0];               // expect: 0
print r[2];               // expect: 2
print r.len();            // expect: 3
