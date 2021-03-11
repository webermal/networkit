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

constexpr double inf = std::numeric_limits<double>::max();

namespace NetworKit {

MinCutStoerWagner::MinCutStoerWagner(const Graph &G) :
		G(&G), current_graph(G, true, false), pq(G.upperNodeIdBound()),
				keys(G.upperNodeIdBound()), s(0), t(0), s_t_cut(false) {
	G.forNodes([&](node u) {
		node_mapping.push_back(u);
	});
}

MinCutStoerWagner::MinCutStoerWagner(const Graph &G, node s, node t) :
		G(&G), current_graph(G, true, false), pq(G.upperNodeIdBound()),
						keys(G.upperNodeIdBound()), s(s), t(t), s_t_cut(true) {
	G.forNodes([&](node u) {
		node_mapping.push_back(u);
	});
}

void MinCutStoerWagner::run() {
	Partition best_solution;
	double best_cut = inf;
	EdgeCut ec;

	while (current_graph.numberOfNodes() > 1) {
		Partition current_solution = phase(s);
		double current_cut = ec.getQuality(current_solution, *G);

		if (current_cut < best_cut && (!s_t_cut || current_solution[s] != current_solution[t])) {
			best_solution = current_solution;
			best_cut = current_cut;
		}
	}

	result = best_solution;
	hasRun = true;
}

void MinCutStoerWagner::fillQueue(node a) {
	current_graph.forNodes([&](node u) {
		if (u == a) {
			return;
		}

		edgeweight weight = current_graph.weight(a, u);
		pq.insert(-weight, u);
		keys[u] = -weight;
	});
}

void MinCutStoerWagner::updateKeys(node u, Partition &A) {
	current_graph.forNeighborsOf(u, [&](node v, edgeweight weight) {
		if (A[v] == 0) {
			pq.changeKey(keys[v] - weight, v);
			keys[v] -= weight;
		}
	});
}

Partition MinCutStoerWagner::phase(node a) {
	Partition A(G->numberOfNodes(), 0);
	Partition result(G->numberOfNodes(), 0);

	if (current_graph.numberOfNodes() == 2) {
		current_graph.forNodes([&](node u) {
			result.moveToSubset((node_mapping[u] == a) ? 1 : 0, u);
		});

		current_graph.removeNode(a);

		return result;
	}

	pq.clear();
	fillQueue(a);
	A[a] = 1;

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
	G->forNodes([&](node u) {
		result.moveToSubset(A[node_mapping[u]], u);
	});

	// update node mapping
	G->forNodes([&](node u){
		if (node_mapping[u] == s) {
			node_mapping[u] = t;
		}
	});

	// shrink G by merging s and t
	current_graph.forNeighborsOf(s, [&](node u, edgeweight weight) {
		if (u != t) {
			current_graph.setWeight(t, u, current_graph.weight(t, u) + weight);
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
