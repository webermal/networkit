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

        bool isDirected;
        Graph flowGraph;

        node s;
        node t;

        std::set<node> A;
        std::set<node> S;
        std::vector<node> S0;
        double f_a;
        index minS;

        count iter;
        count maxIter;

        double relativeQuotientScore(const Partition& p);
        void updateEdgeWeights(double alpha);

    public:

        ImproveClustering(const Graph &G, const Partition initPartition, const count maxIter);
        void run() override;
    };
}

#endif // NETWORKIT_IMPROVECLUSTERING_HPP
