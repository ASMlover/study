
foreign class Random {
  construct new() {
    seed()
  }

  construct new(seed_val) {
    if (seed_val is Numeric) {
      seed(seed_val)
    } else if (seed_val is Sequence) {
      if (seed_val.isEmpty) Fiber.abort("sequence cannot be empty")

      var seeds = []
      for (element in seed) {
        if (!(element is Numeric)) Fiber.abort("sequence elements must all be numbers")

        seeds.add(element)
        if (seeds.count == 16) break
      }

      // cycle the values to fill in any missing slots
      var i = 0
      while (seeds.count < 16) {
        seeds.add(seeds[i])
        i = i + 1
      }

      seed(
        seeds[0], seeds[1], seeds[2], seeds[3],
        seeds[4], seeds[5], seeds[6], seeds[7],
        seeds[8], seeds[9], seeds[10], seeds[11],
        seeds[12], seeds[13], seeds[14], seeds[15])
    } else {
      Fiber.abort("seed must be a number or a sequence of numbers")
    }
  }

  foreign seed()
  foreign seed(seed_val)
  foreign seed(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16)

  foreign float()
  float(max) { float() * max }
  float(min, max) { float() * (max - min) + min }

  foreign int()
  int(max) { (float() * max).floor }
  int(min, max) { (float() * (max - min)).floor + min }
}
