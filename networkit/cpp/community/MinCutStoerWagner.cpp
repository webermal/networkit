/*
 * MinCutStoerWagner.cpp
 *
 *  Created on: 01.03.2021
 *      Author: Johannes Friedrich Lange
 */

#include <sstream>

#include <iostream>

#include <networkit/community/MinCutStoerWagner.hpp>
#include <networkit/community/EdgeCut.hpp>
#include <networkit/graph/GraphTools.hpp>

constexpr int64_t inf = std::numeric_limits<int64_t>::max();
constexpr int64_t neg_inf = std::numeric_limits<int64_t>::min();

namespace NetworKit {

MinCutStoerWagner::MinCutStoerWagner(const Graph &G) :
		G(&G), current_graph(G, true, false), pq(
				G.upperNodeIdBound(), -GraphTools::maxWeightedDegree(G), 0) {
	G.forNodes([&](node u) {
		node_mapping.push_back(u);
	});
}

void MinCutStoerWagner::run() {
	Partition best_solution;
	double best_cut = inf;
	EdgeCut ec;

	while (current_graph.numberOfNodes() > 1) {
		Partition current_solution = phase(0);
		double current_cut = ec.getQuality(current_solution, *G);

		if (current_cut < best_cut) {
			best_solution = current_solution;
			best_cut = current_cut;
		}
	}

	result = best_solution;
	hasRun = true;
}

void MinCutStoerWagner::fillQueue(node a) {
	current_graph.forNodes([&](node u) {
		edgeweight weight = current_graph.weight(a, u);

		if (weight > 0) {
			pq.insert(-weight, u);
		} else {
			pq.insert(neg_inf, u);
		}
	});
}

void MinCutStoerWagner::clearQueue() {
	while (pq.size() > 0) {
		pq.extractMin();
	}
}

void MinCutStoerWagner::updateKeys(node u, Partition &A) {
	current_graph.forNeighborsOf(u, [&](node v, edgeweight weight) {
		if (A[v] == 0) {
			pq.changeKey(pq.getKey(v) - weight, v);
		}
	});
}

Partition MinCutStoerWagner::phase(node a) {
	Partition A(G->numberOfNodes(), 0);
	Partition result(G->numberOfNodes(), 0);
	A[a] = 1;

	clearQueue();
	fillQueue(a);

	while (pq.size() > 2) {
		node u = pq.extractMin().second;
		A[u] = 1;
		updateKeys(u, A);
	}

	node s = pq.extractMin().second;
	A[s] = 1;
	updateKeys(s, A);
	node t = pq.extractMin().second;

	// transform A to fit on the whole graph G
	current_graph.forNodes([&](node u) {
		result[u] = A[node_mapping[u]];
	});

	node_mapping[s] = t;

	// shrink G by merging s and t
	current_graph.forNeighborsOf(s,
			[&](node u, edgeweight weight) {
				if (u != t) {
					current_graph.setWeight(t, u,
							current_graph.weight(t, u) + weight);
				}
			});

	current_graph.removeNode(s);

	return result;
}

Partition MinCutStoerWagner::getPartition() {
	if (!hasRun) {
		throw std::runtime_error("Call run()-function first.");
	}
	return result;
}

std::string MinCutStoerWagner::toString() const {
	std::stringstream stream;
	stream << "MinCutStoerWagner()";

	return stream.str();
}

} /* namespace NetworKit */
