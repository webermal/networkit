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

constexpr double inf = std::numeric_limits<int64_t>::max();

namespace NetworKit {

MinCutStoerWagner::MinCutStoerWagner(const Graph &G) :
		G(&G), current_graph(G, true, false), pq(G.upperNodeIdBound(), -G.totalEdgeWeight(), 0) {
	G.forNodes([&](node u) {
		node_mapping.push_back(u);
	});
	G.forEdges([&](node u, node v, edgeweight weight) {
		assert((int) weight == weight);
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

//		std::cout << "PHASE " << current_cut << "\n";
//		std::cout << "PART ";
//		G->forNodes([&](node u){
//			std::cout << current_solution[u];
//		});
//		std::cout << "\n";
	}

//	std::cout << "PART ";
//	G->forNodes([&](node u){
//		std::cout << best_solution[u];
//	});
//	std::cout << "\n";

	result = best_solution;
	hasRun = true;
}

void MinCutStoerWagner::fillQueue(node a) {
	current_graph.forNodes([&](node u) {
		if (u == a) {
			return;
		}

		edgeweight weight = current_graph.weight(a, u);
		pq.insert((int64_t) (-weight), u);
	});
}

void MinCutStoerWagner::updateKeys(node u, Partition &A) {
	current_graph.forNeighborsOf(u, [&](node v, edgeweight weight) {
		if (A[v] == 0) {
			pq.changeKey((int64_t) (pq.getKey(v) - weight), v);
		}
	});
}

Partition MinCutStoerWagner::phase(node a) {
	Partition A(G->numberOfNodes(), 0);
	Partition result(G->numberOfNodes(), 0);

	if (current_graph.numberOfNodes() == 2) {
//		current_graph.forNodes([&](node u){
//			std::cout << u << "\n";
//		});

		result.moveToSubset(1, a);
		current_graph.removeNode(a);

		return result;
	}

	pq.clear();
	fillQueue(a);
	A[a] = 1;

//	std::cout << "INITIAL ";
//	current_graph.forNodes([&](node u){
//		if (current_graph.hasNode(u)) std::cout << u << ":" << -pq.getKey(u) << " ";
//	});
//	std::cout << "\n";

	while (pq.size() > 2) {
		node u = pq.extractMin().second;
		A[u] = 1;
//		std::cout << "move " << u << "\n";
		updateKeys(u, A);

//		current_graph.forNodes([&](node u){
//			if (current_graph.hasNode(u)) std::cout << u << ":" << -pq.getKey(u) << " ";
//		});
//		std::cout << "\n";
	}

	node s = pq.extractMin().second;
	A[s] = 1;
	updateKeys(s, A);
	node t = pq.extractMin().second;

//	std::cout << "s=" << s << ";t=" << t << "\n";

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
