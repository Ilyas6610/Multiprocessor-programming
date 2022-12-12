#include "algos.h"
#include "profiler.h"
#include <assert.h>

int main() {
	Graph g = makeGraph();
	GraphInt<int> p1(N, std::vector(N, std::vector<int>(N, 0)));;
	GraphInt<std::shared_ptr<std::atomic<int>>> p2(N, 
		std::vector(N, 
			std::vector<std::shared_ptr<std::atomic<int>>>(N)));
	for (int i = 0; i < p2.size(); i++) {
		for (int j = 0; j < p2[i].size(); j++) {
			for (int k = 0; k < p2[j].size(); k++) {
				p2[i][j][k] = std::make_shared<std::atomic<int>>(0);
			}
		}
	}
	Graph g1 = g;
	{
		std::cout << "Sequential ";
		Profiler p;
		bfs(g1, p1);
	}
	Graph g2 = g;
	{
		std::cout << "Parallel ";
		Profiler p;
		tbb::task_arena arena(4);

		arena.execute([&] { nd_bfs(g2, p2); });
	}

	for (int i = 0; i < p1.size(); i++) {
		for (int j = 0; j < p1[i].size(); j++) {
			for (int k = 0; k < p1[j].size(); k++) {
				assert(p1[i][j][k] == p2[i][j][k]->load());
			}
		}
	}
	return 0;
}