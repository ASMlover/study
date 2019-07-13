
var f0 = fn 0
IO.write(f0.call) // expect: 0

var f1 = fn(a) a
IO.write(f1.call(1)) // expect: 1

var f2 = fn(a,b) a+b
IO.write(f2.call(1,2)) // expect: 3

var f3 = fn(a,b,c) a+b+c
IO.write(f3.call(1,2,3)) // expect: 6

var f4 = fn(a,b,c,d) a+b+c+d
IO.write(f4.call(1,2,3,4)) // expect: 10

var f5 = fn(a,b,c,d,e) a+b+c+d+e
IO.write(f5.call(1,2,3,4,5)) // expect: 15

var f6 = fn(a,b,c,d,e,f) a+b+c+d+e+f
IO.write(f6.call(1,2,3,4,5,6)) // expect: 21

var f7 = fn(a,b,c,d,e,f,g) a+b+c+d+e+f+g
IO.write(f7.call(1,2,3,4,5,6,7)) // expect: 28

var f8 = fn(a,b,c,d,e,f,g,h) a+b+c+d+e+f+g+h
IO.write(f8.call(1,2,3,4,5,6,7,8)) // expect: 36

var f9 = fn(a,b,c,d,e,f,g,h,i) a+b+c+d+e+f+g+h+i
IO.write(f9.call(1,2,3,4,5,6,7,8,9)) // expect: 45

var fa = fn(a,b,c,d,e,f,g,h,i,j) a+b+c+d+e+f+g+h+i+j
IO.write(fa.call(1,2,3,4,5,6,7,8,9,10)) // expect: 55

var fb = fn(a,b,c,d,e,f,g,h,i,j,k) a+b+c+d+e+f+g+h+i+j+k
IO.write(fb.call(1,2,3,4,5,6,7,8,9,10,11)) // expect: 66

var fc = fn(a,b,c,d,e,f,g,h,i,j,k,l) a+b+c+d+e+f+g+h+i+j+k+l
IO.write(fc.call(1,2,3,4,5,6,7,8,9,10,11,12)) // expect: 78

var fd = fn(a,b,c,d,e,f,g,h,i,j,k,l,m) a+b+c+d+e+f+g+h+i+j+k+l+m
IO.write(fd.call(1,2,3,4,5,6,7,8,9,10,11,12,13)) // expect: 91

var fe = fn(a,b,c,d,e,f,g,h,i,j,k,l,m,n) a+b+c+d+e+f+g+h+i+j+k+l+m+n
IO.write(fe.call(1,2,3,4,5,6,7,8,9,10,11,12,13,14)) // expect: 105

var ff = fn(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) a+b+c+d+e+f+g+h+i+j+k+l+m+n+o
IO.write(ff.call(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15)) // expect: 120

var f10 = fn(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p
IO.write(f10.call(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)) // expect: 136
