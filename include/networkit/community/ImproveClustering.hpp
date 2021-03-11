//
// Created by malte on 26.02.21.
//

#ifndef NETWORKIT_IMPROVECLUSTERING_HPP
#define NETWORKIT_IMPROVECLUSTERING_HPP
#include <networkit/graph/Graph.hpp>
#include <networkit/community/CommunityDetectionAlgorithm.hpp>

namespace NetworKit {
    class ImproveClustering final: public CommunityDetectionAlgorithm {

    private:
        Graph flowGraph;
        node s;
        node t;
        std::set<node> initPartitionSet1;

        double relativeQuotientScore(const Graph* G, std::set<node> A, std::set<node> S);

        Partition initPartition;

    public:

        ImproveClustering(const Graph &G, const Partition initPartition);
        void run() override;
    };
}

#endif // NETWORKIT_IMPROVECLUSTERING_HPP
