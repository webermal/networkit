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
        //Graph FlowGraph;
        //node s;
        //node t;
        Partition initPartition;
        float relativeQuotientScore(const Graph *G, std::set<node>, std::set<node>);
        //void updateEdgeWeights(Graph &G, float alpha, Partition A);

    public:

        ImproveClustering(const Graph &G, const Partition &initPartition);
        void run() override;


    };
}

#endif // NETWORKIT_IMPROVECLUSTERING_HPP
