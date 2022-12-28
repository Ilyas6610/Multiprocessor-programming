#include "algos.h"
#include "profiler.h"
#include <assert.h>

int main() {
	srand(42);

	std::vector<int> v;

	for (int i = 0; i < 10000; ++i) {
		if (rand() % 2)
			v.push_back(i);
	}

	int nProc, x;
	std::cin >> nProc >> x;
	std::cout << nProc << " proc, ";
	int ops = 0;
	for (int k = 0; k <= 4; k++)
		ops += testNProcesses(nProc, x, v);

	std::cout << "x = " << x << ' ' << ops / 25 << " operations per second" << std::endl;


	return 0;
}