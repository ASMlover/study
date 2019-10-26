
// newline after `?`
IO.print(true ?
  "yes" : "no") // expect: yes

// newline after `:`
IO.print(false ? "yes" :
  "no") // expect: no
