/*
 * MinCutStoerWagner.hpp
 *
 *  Created on: 01.03.2021
 *      Author: Johannes Friedrich Lange
 */

#ifndef NETWORKIT_COMMUNITY_MIN_CUT_STOER_WAGNER_HPP_
#define NETWORKIT_COMMUNITY_MIN_CUT_STOER_WAGNER_HPP_

#include <networkit/base/Algorithm.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/structures/Partition.hpp>
#include <networkit/auxiliary/BucketPQ.hpp>

namespace NetworKit {

/**
 */
class MinCutStoerWagner final : public Algorithm {

public:
    /**
     * @param[in] G input graph
     */
    MinCutStoerWagner(const Graph& G);

    /**
     * Get string representation.
     *
     * @return String representation of this algorithm.
     */
    std::string toString() const override;

    /**
     * Detect communities.
     */
    void run() override;

    /**
	 * Returns the result of the run method or throws an error, if the algorithm hasn't run yet.
	 * @return partition of the node set
	 */
	Partition getPartition();

private:
    
    const Graph* G;

    Graph current_graph;
    std::vector<node> node_mapping;
    Aux::BucketPQ pq;
    Partition result;

    void fillQueue(node a);
    void updateKeys(node u, Partition& A);

    Partition phase(node a);

};

} /* namespace NetworKit */

#endif // NETWORKIT_COMMUNITY_MIN_CUT_STOER_WAGNER_HPP_
