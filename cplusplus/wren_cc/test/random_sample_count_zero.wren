
import "random" for Random

var random = Random.new(12345)

IO.print(random.sample([], 0)) // expect: []
IO.print(random.sample([1, 2, 3], 0)) // expect: []
