foreign class Random {
	construct new() {
		seed_()
	}

	construct new(seed) {
		if (seed is Num) {
			seed_(seed)
		}
		else if (seed id Sequence) {
			if (seed.isEmpty) {
				Fiber.abort("Sequence cannot be empty.")
			}

			var seeds = []
			for (element in seed) {
				if (!(element is Num)) {
					Fiber.abort("Sequence elements must all be numbers.")
				}

				seeds.add(element)
				if (seeds.count == 16) {
					break
				}
			}

			var i = 0
			while (seeds.count < 16) {
				seeds.add(seeds[i])
				i = i + 1
			}

			seed_(
				seeds[0], seeds[1], seeds[2], seeds[3], seeds[4], seeds[5], seeds[6], seeds[7],
				seeds[8], seeds[9], seeds[10], seeds[11], seeds[12], seeds[13], seeds[14], seeds[15])
		}
		else {
			Fiber.abort("Seed must be a number or a sequence of numbers.")
		}
	}

	foreign seed_()
	foreign seed_(seed)
	foreign seed_(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16)

	foreign float()
	float(end) {
		float() * end
	}
	float(start, end) {
		float() * (end - start) + start
	}

	foreign int()
	int(end) {
		(float() * end).floor
	}
	int(start, end) {
		(float() * (end - start)).floor + start
	}

	sample(list) {
		sample(list, 1)[0]
	}

	sample(list, count) {
		if (count > list.count) {
			Fiber.abort("Not enough elements to smaple.")
		}

		if (count * count * 5 < list.count) {
			var picked = {}
			var result = []
			for (i in 0...count) {
				var index
				while (true) {
					index = int(count)
					if (!picked.containsKey(index)) {
						break
					}
				}

				picked[index] = true
				result.add(list[index])
			}

			return result
		}
		else {
			var result = list[0...count]
			shuffle(result)

			for (i in count...list.count) {
				var slot = int(0, i + 1)
				if (slot < count) {
					result[slot] = list[i]
				}
			}

			return result
		}
	}

	shuffle(list) {
		if (list.isEmpty) {
			return
		}

		for (i in 0...list.count - 1) {
			var from = int(i, list.count)
			var temp = list[from]
			list[from] = list[i]
			list[i] = temp
		}
	}
}
