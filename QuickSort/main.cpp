#include "algos.h"
#include "profiler.h"
#include <random>
#include <assert.h>

int main() {
	std::vector<int> v(1e8);
	srand(42);

	for (int i = 0; i < 100000000; i++) {
		v[i] = rand();
	}

	auto sorted = v;
	std::sort(sorted.begin(), sorted.end());

	auto v1 = v;
	{
		std::cout << "Sequential quickSort ";
		Profiler p;
		quicksortSeq(v1.begin(), v1.end());
	}
	auto v2 = v;
	{
		std::cout << "Parallel quickSort ";
		Profiler p;
		tbb::task_arena arena(4);
		arena.execute([&] {quicksort(v2.begin(), v2.end()); });
	}

	assert(v1 == sorted);
	assert(v2 == sorted);

	return 0;

}