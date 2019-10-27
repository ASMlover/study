
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
      for (element in seed_val) {
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

  sample(list) { sample(list, 1)[0] }
  sample(list, count) {
    if (count > list.len) Fiber.abort("not enough elements to sample")

    // there at (at least) two simple algorithms for choosing a number of
    // samples from a list without replcement -- where we do not pick the
    // same element more that one
    //
    // the first is faster when the number of samples is small relative to
    // the size of the collection, in many cases, it avoids scanning the
    // entire list, in the common case of just wanting one sample, it is a
    // single random index lookup
    //
    // however, its performance degrades badlu as the sample size increases
    // Vitter's algorithm always scans the entire list, but it's alse always
    // O(n)
    //
    // the cutoff point between the two follows a quadratic curve on the
    // same size, based on some empirical testing, scaling that by 5 seems
    // to fit pretty closely and chooses the fastest one for the given
    // sample and collection size

    if (count * count * 5 < list.len) {
      // pick random elements and retry if you hit a previously chosen one
      var picked = {}
      var result = []
      for (i in 0...count) {
        // find an index that we have not already selected
        var index
        while (true) {
          index = int(count)
          if (!picked.containsKey(index)) break
        }

        picked[index] = true
        result.add(list[index])
      }

      return result
    } else {
      // Jeffret Vitter's algorithm R

      // fill the reservoir with the first elements in the list
      var result = list[0...count]

      // we want to ensure the results are always in random order, so
      // shuffle them, in cases where the sample size is the entire
      // collection, this devolves to running Fisher-Yates on a copy of
      // the list
      shuffle(result)

      // now walk the rest of the list, for each element, randomly consider
      // replacing one of the reservoir elements with it, the probability
      // here works out such that it does this uniformly
      for (i in count...list.len) {
        var slot = int(0, i + 1)
        if (slot < count) result[slot] = list[i]
      }

      return result
    }
  }

  shuffle(list) {
    if (list.isEmpty) return

    // fisher-yates shuffle
    for (i in 0...list.len - 1) {
      var from = int(i, list.len)
      var temp = list[from]
      list[from] = list[i]
      list[i] = temp
    }
  }
}
