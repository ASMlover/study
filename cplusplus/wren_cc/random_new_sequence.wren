
import "random" for Random

var random1 = Random.new([1, 2, 3])
var random2 = Random.new([1, 2, 3])

var correct = 0
for (i in 1..100) {
  if (random1.float() == random2.float()) correct = correct + 1
}

IO.print(correct) // expect: 100
