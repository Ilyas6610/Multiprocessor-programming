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
#include <assert.h>

struct Node {
	Node(int _val, bool _isLeaf) : val(_val), isLeaf(_isLeaf) {};

	tbb::spin_mutex mutex;
	Node* left = nullptr;
	Node* right = nullptr;
	int val;
	bool isLeaf;

	void lock() {
		mutex.lock();
	}

	void unlock() {
		mutex.unlock();
	}
};

class Tree {
public:
	Node* root;
	tbb::spin_mutex m;

	Tree(Node* _root) : root(_root) {};

	bool insert(int val) {
		m.lock();
		if (!root) {
			root = new Node(val, true);
			m.unlock();
			return true;
		}

		Node* newNode = new Node(val, true);
		root->lock();
		m.unlock();
		Node* cur = root;
		Node* par = nullptr;
		while (true) {
			if (cur->isLeaf)
				break;
			if (cur->val > val)
			{
				cur->left->lock();
				cur->unlock();	
				cur = cur->left;
			}
			else if (cur->val < val)
			{
				cur->right->lock();
				cur->unlock();
				cur = cur->right;
			}
			else {
				cur->unlock();
				return false;
			}
		}
		cur->isLeaf = false;

		Node *right, *left;
		if (cur->val < val) {
			right = new Node(val, true);
			left = new Node(cur->val, true);
			cur->val = val;
		}
		else {
			left = new Node(val, true);
			right = new Node(cur->val, true);
		}
		
		cur->left = left;
		cur->right = right;
		cur->unlock();
		return true;
	}

	bool remove(int val) {
		m.lock();
		if (root == nullptr) {
			m.unlock();
			return false;
		}

		if (root->isLeaf) {
			if (val == root->val) {
				delete root;
				root = nullptr;
				m.unlock();
				return true;
			}
			else {
				m.unlock();
				return false;
			}
		}

		root->lock();
		m.unlock();

		Node* par = root, * prePar = root,
			* cur = root, * route = nullptr;

		while (true) {
			if (cur->isLeaf && cur->val != val)
			{
				cur->unlock();
				par->unlock();
				prePar->unlock();
				if(route) route->unlock();

				return false;
			}
			if (cur->val == val && !cur->isLeaf) {
				route = cur;
			}
			if (cur->val > val) {
				if (prePar != par && prePar != route)
					prePar->unlock();
				prePar = par;
				par = cur;
				cur->left->lock();
				cur = cur->left;
			}
			else if (cur->val < val || (cur->val == val && !cur->isLeaf)) {
				if (prePar != par && prePar != route)
					prePar->unlock();
				prePar = par;
				par = cur;        
				cur->right->lock();
				cur = cur->right;
			}
			else{
				if (par->val != cur->val)
					if(route)
						route->val = par->val;
				if (prePar == root && par == prePar) {
					if (par->left == cur) {
						par = par->right;
						root = par;
					}
					else if (par->right == cur) {
						par = par->left;
						root = par;
					}

					cur->unlock();
					par->unlock();
					if (route) route->unlock();

					delete cur;

					return true;
				}
				else if (prePar->left == par) {
					if (par->left == cur) {
						prePar->left = par->right;
					}
					else {
						prePar->left = par->left;
					}
				}
				else {
					if (par->left == cur) {
						prePar->right = par->right;
					}
					else {
						prePar->right = par->left;
					}
				}

				cur->unlock();
				par->unlock();
				prePar->unlock();
				if (route) route->unlock();
				delete par, cur;

				return true;
			}
		}
	}

	bool contains(int val) {
		Node* cur = root;
		while (true) {
			if (cur == nullptr)
			{
				return false;
			}

			if (cur->val > val)
				cur = cur->left;
			else if (cur->val < val)
				cur = cur->right;
			else {
				return true;
			}
		}
	}

};


void dfs(Node* node, std::vector<std::pair<int,int>>& v, int &counter, int &curSum) {
	if (node == nullptr)
		return;
	if (node->left != nullptr) {
		dfs(node->left, v, counter, curSum);
	}

	v.push_back({ node->val, node->isLeaf });
	if (node->isLeaf) {
		counter++;
		curSum += node->val;
	}

	if (node->right != nullptr) {
		dfs(node->right, v, counter, curSum);
	}
}

std::pair<int, int> run(std::vector<int>& K, std::atomic<int>& op, Tree& tree, int proc, int k) {
	std::atomic<int> val(0), sum(0);
	auto func = [&](std::chrono::system_clock::time_point begin = std::chrono::system_clock::now()) {
		size_t time = 0;
		int x = k;
		while (time < 5) {
			int key = K[rand() % K.size()];
			int p = rand()%100;
			if (p < x)
			{
				if (tree.insert(key)) {
					val++;
					sum.fetch_add(key);
				}
			}
			else if (p >= x && p < 2 * x)
			{
				if (tree.remove(key)) {
					val--;
					sum.fetch_add(-key);
				}
			}
			else if (p >= 2 * x && p < 100)
			{
				tree.contains(key);
			}

			op.fetch_add(1);
			time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - begin).count();
		}
	};

	switch (proc) {
		case 1:
			func();
			break;
		case 2:
			tbb::parallel_invoke([&] { func(); }, [&] {func(); });
			break;
		case 3:
			tbb::parallel_invoke([&] { func(); }, [&] {func(); }, [&] {func(); });
			break;
		case 4:
			tbb::parallel_invoke([&] { func(); }, [&] {func(); }, [&] {func(); }, [&] {func(); });
			break;
	}
	
	return { val.load(), sum.load() };
}

bool checkTree(Tree& tree, int size, int sum) {
	std::vector<std::pair<int, int>> v;
	int counter = 0, curSum = 0;
	dfs(tree.root, v, counter, curSum);
	if (counter != size || curSum != sum)
		return false;
	for (int i = 1; i < v.size(); i++) {
		if (v[i - 1].first > v[i].first)
			return false;
	}

	return true;
}

int testNProcesses(int nProc, int x, std::vector<int>& v) {
	Tree tree(nullptr);
	std::atomic<int> ops(0);
	tbb::task_arena arena(nProc);

	int size, sum;
	arena.execute(
		[&]() {
			auto p = run(v, ops, tree, nProc, x);
			size = p.first;
			sum = p.second;
		}
	);
	assert(checkTree(tree, size, sum));

	return ops.load();
}