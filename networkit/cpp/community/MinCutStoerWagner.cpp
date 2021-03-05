/*
 * MinCutStoerWagner.cpp
 *
 *  Created on: 01.03.2021
 *      Author: Johannes Friedrich Lange
 */

#include <sstream>

#include <networkit/community/MinCutStoerWagner.hpp>
#include <networkit/graph/GraphTools.hpp>

namespace NetworKit {

MinCutStoerWagner::MinCutStoerWagner(const Graph &G) :
		G(G), current_graph(&G), pq(
				G.upperNodeIdBound(),
				-NetworKit::GraphTools::maxWeightedDegree(G),
				NetworKit::GraphTools::maxWeightedDegree(G)) {
	G.forNodes([&](node u) {
		node_mapping.push_back(u);
	});
}

void MinCutStoerWagner::run() {
	Partition best_solution;
	double best_cut = 0;

	while (current_graph->numberOfNodes() > 1) {
		Partition current_solution = phase(0);
		double current_cut = cut(current_solution);

		if (current_cut > best_cut) {
			best_solution = current_solution;
			best_cut = current_cut;
		}
	}

	result = best_solution;
	hasRun = true;
}

double MinCutStoerWagner::cut(Partition& p) {
	double cut = 0;

	G->forEdges([&](node u, node v, edgeweight weight) {
		if (p[u] != p[v]) {
			cut += weight;
		}
	});

	return cut;
}

double MinCutStoerWagner::getKey(node u) {
	double cut = 0;

	current_graph->forNeighborsOf(u, [&](node v, edgeweight weight) {
		cut += weight;
	});

	return -cut;
}

void MinCutStoerWagner::fillQueue() {
	current_graph->forNodes([&](node u) {
		pq.insert(getKey(u), u);
	});
}

void MinCutStoerWagner::updateKeys(node u, Partition& A) {
	G->forNeighborsOf(u, [&](node v, edgeweight weight) {
		if (A[v] == 0) {
			pq.changeKey(pq.getKey(v) + weight, v);
		}
	});
}

Partition MinCutStoerWagner::phase(node a) {
	Partition A(current_graph->numberOfNodes());
	Partition result(current_graph->numberOfNodes());
	A[a] = 1;

	while (A.subsetSizes()[0] > 2) {
		node u = pq.extractMin();
		A[u] = 1;
		updateKeys(u, A);
	}

	node s = pq.extractMin();
	A[s] = 1;
	updateKeys(s, A);
	node t = pq.extractMin();

	// transform A to fit on the whole graph G
	G->forNodes([&](node u) {
		result[u] = A[node_mapping[u]];
	});

	node_mapping[s] = t;

	// shrink current_graph by merging s and t
	current_graph->forNeighborsOf(s, [&](node u, edgeweight weight) {
		current_graph->setWeight(t, u, current_graph->weight(t, u) + weight);
	});

	current_graph->removeNode(s);

	return result;
}

Partition MinCutStoerWagner::getPartition() {
	if(!hasRun) {
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
