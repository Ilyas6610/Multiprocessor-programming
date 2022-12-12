#pragma once
#include <tbb/tbb.h>
#include <queue>
#include <vector>
#include <map>
#include <numeric>
#include <iostream>
#include <execution>
#include <algorithm>
#include "profiler.h"
#define N 200

struct Node {
	int x, y, z;
};

std::atomic<int> counter(0);
int counter_seq(0);
long long t = 0;

using Graph = std::vector < std::vector < std::vector<std::vector<Node>>>>;
template <class T>
using GraphInt = std::vector < std::vector < std::vector<T>>>;

Graph makeGraph() {
	Graph g(N, std::vector(N, std::vector(N, std::vector<Node>())));
	for (int i = 0; i < N-1; i++) {
		for (int j = 0; j < N-1; j++) {
			for (int k = 0; k < N-1; k++) {
				g[i][j][k].push_back({ i + 1, j, k });
				g[i][j][k].push_back({ i, j + 1, k });
				g[i][j][k].push_back({ i, j, k + 1 });
				g[i][j][k].push_back({ i, j + 1, k + 1 });
				g[i][j][k].push_back({ i + 1, j, k + 1 });
				g[i][j][k].push_back({ i + 1, j, k + 1 });
				g[i][j][k].push_back({ i + 1, j + 1, k + 1 });
			}
		}
	}
	return g;
}

void bfs(Graph& g, GraphInt<int>& paths) {
	std::queue<Node> q;

	q.push({ 0, 0, 0 });
	int len = 0;
	while (!q.empty()) {
		int sz = q.size();
		++len;
		for (int i = 0; i < sz; i++) {
			auto node = q.front();
			q.pop();
			auto children = g[node.x][node.y][node.z];
			for (int j = 0; j < children.size(); j++) {
				if (!paths[children[j].x][children[j].y][children[j].z]) {
					paths[children[j].x][children[j].y][children[j].z] = len;
					q.push(children[j]);
				}
			}
		}
	}
};

void nd_bfs(Graph& g, GraphInt<std::shared_ptr<std::atomic<int>>>& paths) {
	std::vector<Node> frontier = { {0, 0, 0} };
	std::vector<int> positions;
	int len = 0;

	while (!frontier.empty()) {
		++len;
		
		positions.resize(frontier.size());
		tbb::parallel_for(tbb::blocked_range<int>(0, frontier.size()),
			[&](tbb::blocked_range<int> r) {
				for (auto i = r.begin(); i != r.end(); i++) {
					auto& node = frontier[i];
					positions[i] = g[node.x][node.y][node.z].size();
				}
			});

		std::inclusive_scan(std::execution::par_unseq, positions.begin(),
			positions.end(), positions.begin());
		std::vector<Node> newFrontier(*(positions.end() - 1), {-1, -1, -1});

		tbb::parallel_for(tbb::blocked_range<int>(0, frontier.size()),
			[&](tbb::blocked_range<int> r) {
				for (auto i = r.begin(); i != r.end(); i++) {
					auto node = frontier[i];
					const std::vector<Node>& children = g[node.x][node.y][node.z];
					int beg = i == 0 ? 0 : positions[i - 1];

					tbb::parallel_for(
						tbb::blocked_range<int>(beg , positions[i]),
						[&](tbb::blocked_range<int> r1) {
							for (int j = r1.begin(); j < r1.end(); j++) {
								int testVal = 0;
								auto& k = children[j-beg];
								if (!paths[k.x][k.y][k.z]->load() &&
									paths[k.x][k.y][k.z]->compare_exchange_strong(testVal, len)) {
									newFrontier[j] = k;
								}
							}
						});

				}
			});
		frontier = std::move(newFrontier);
		auto it = std::remove_if(std::execution::par_unseq, frontier.begin(), frontier.end(),
			[](Node& n) {
				return n.x == -1 && n.y == -1 && n.z == -1;
			});
		frontier.erase(it, frontier.end());
	}

};