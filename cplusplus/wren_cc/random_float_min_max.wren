
import "random" for Random

var random = Random.new(12345)

for (i in 1..1000) {
  var n = random.float(2, 5)
  if (n < 2) IO.print("too low")
  if (n >= 5) IO.print("too high")
}
