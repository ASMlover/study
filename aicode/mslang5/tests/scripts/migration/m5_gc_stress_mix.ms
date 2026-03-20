class Node {
  init(v, next) {
    this.v = v
    this.next = next
  }
}

fun build_chain(limit) {
  var head = nil
  var i = 0
  while (i < limit) {
    head = Node(i, head)
    i = i + 1
  }
  return head
}

fun sum_prefix(node, take) {
  var total = 0
  var i = 0
  var cur = node
  while (i < take and cur != nil) {
    total = total + cur.v
    cur = cur.next
    i = i + 1
  }
  return total
}

var keep = build_chain(160)
var churn = 0
while (churn < 40) {
  build_chain(30)
  churn = churn + 1
}

print sum_prefix(keep, 5)