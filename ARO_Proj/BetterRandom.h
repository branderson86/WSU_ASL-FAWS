////////////////////
// BetterRandom.h - project's randomizer
////////////////////

#ifndef BETTER_RANDOM_H_
#define BETTER_RANDOM_H_

#include <random>
#include <mutex>

// This class acts as a simpler interface to the much more random & capable random number generator -> random_device.
struct BetterRandom {
	std::random_device rd;
	std::mt19937 *mt;
	std::uniform_int_distribution<int> *dist;

	// Default use max
	BetterRandom() : BetterRandom(RAND_MAX) {}

	// Constructor with given cap (also used by default)
	BetterRandom(int cap) {
		mt = new std::mt19937(rd());
		dist = new std::uniform_int_distribution<int>(0, cap - 1);
	}

	~BetterRandom() {
		delete mt;
		delete dist;
	}

	// () operator, use this to get a random number
	const int operator()() {
		int result = (*dist)(*mt);
		return result;
	}
};

#endif
