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

#include <networkit/auxiliary/Timer.hpp>

constexpr double inf = std::numeric_limits<int64_t>::max();

namespace NetworKit {

MinCutStoerWagner::MinCutStoerWagner(const Graph &G) :
		G(&G), current_graph(G, true, false), pq(G.upperNodeIdBound(),
				-G.totalEdgeWeight(), 0) {
	G.forNodes([&](node u) {
		node_mapping.push_back(u);
	});
	G.forEdges([&](node u, node v, edgeweight weight) {
		assert((int ) weight == weight);
	});
}

void MinCutStoerWagner::run() {
	Aux::Timer timer, phaseTimer;
	timer.start();
	Partition best_solution;
	double best_cut = inf;
	EdgeCut ec;

	while (current_graph.numberOfNodes() > 1) {
		phaseTimer.start();
		Partition current_solution = phase(0);
		phaseTimer.stop();
		phaseTime += phaseTimer.elapsedMicroseconds();
		double current_cut = ec.getQuality(current_solution, *G);

		if (current_cut < best_cut) {
			best_solution = current_solution;
			best_cut = current_cut;
		}
	}

	result = best_solution;
	hasRun = true;
	timer.stop();
	double algTime = timer.elapsedMicroseconds();

//	std::cout << "Alg. running time: " << algTime / 1000000.0 << " s\n"
//				<< "Phase time: " << phaseTime / 1000000.0 << " s; " << phaseTime / algTime * 100 << " %\n"
//				<< "Initialisation time: " << initTime / 1000000.0 << " s; " << initTime / algTime * 100 << " %\n"
//				<< "Update time: " << updateTime / 1000000.0 << " s; " << updateTime / algTime * 100 << " %\n"
//				<< "Transformation time: " << transformTime / 1000000.0 << " s; " << transformTime / algTime * 100 << " %\n";
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
	Aux::Timer timer;
	timer.start();
	Partition A(G->numberOfNodes(), 0);
	Partition result(G->numberOfNodes(), 0);

	if (current_graph.numberOfNodes() == 2) {
		result.moveToSubset(1, a);
		current_graph.removeNode(a);
		return result;
	}

	pq.clear();
	fillQueue(a);
	A[a] = 1;
	timer.stop();
	initTime += timer.elapsedMicroseconds();

	timer.start();
	while (pq.size() > 2) {
		node u = pq.extractMin().second;
		A[u] = 1;
		updateKeys(u, A);
	}
	timer.stop();
	updateTime += timer.elapsedMicroseconds();

	timer.start();
	node s = pq.extractMin().second;
	A[s] = 1;
	updateKeys(s, A);
	node t = pq.extractMin().second;

	// transform A to fit on the whole graph G
	G->forNodes([&](node u) {
		result.moveToSubset(A[node_mapping[u]], u);
	});

	// update node mapping
	G->forNodes([&](node u) {
		if (node_mapping[u] == s) {
			node_mapping[u] = t;
		}
	});

	timer.start();
	// shrink G by merging s and t
	current_graph.forNeighborsOf(s, [&](node u, edgeweight weight) {
		if (u != t) {
			current_graph.setWeight(t, u, current_graph.weight(t, u) + weight);
		}
	});

	current_graph.removeNode(s);
	timer.stop();
	transformTime += timer.elapsedMicroseconds();

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
