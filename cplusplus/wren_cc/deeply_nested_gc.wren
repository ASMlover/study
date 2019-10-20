
var head

for (i in 1..40000) {
  head = {"next": head}
}

IO.print("done") // expect: done
