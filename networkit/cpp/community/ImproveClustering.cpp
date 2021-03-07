//
// Created by malte on 26.02.21.
//

#include "networkit/community/ImproveClustering.hpp"
#include <networkit/graph/Graph.hpp>
#include "networkit/community/MinCutStoerWagner.hpp"


namespace NetworKit {

    ImproveClustering::ImproveClustering(const Graph& G, const Partition &initPartition): CommunityDetectionAlgorithm(G, initPartition) {}

    void ImproveClustering::run() {

        std::map<index, count> sizes = initPartition.subsetSizeMap();
        index s = std::min_element(sizes.begin(), sizes.end(),[](const auto &l, const auto &r){ return l.second < r.second;})->first;
        float alpha_0 = relativeQuotientScore(G, initPartition.getMembers(s), initPartition.getMembers(s));
        float alpha;
        int i = 0;
        do {
            i += 1;
            alpha = alpha_0;

            MinCutStoerWagner minCut = MinCutStoerWagner(*G);
            minCut.run();
            Partition minCutPartition = minCut.getPartition();

            std::map<index, count> minCutSizes = minCutPartition.subsetSizeMap();
            index minS = std::min_element(minCutSizes.begin(), minCutSizes.end(),[](const auto &l, const auto &r){ return l.second < r.second;})->first;
            alpha_0 = relativeQuotientScore(G, initPartition.getMembers(s), minCutPartition.getMembers(minS));

            result = minCutPartition;

        } while (alpha_0 < alpha and i < 10);
    }

    float ImproveClustering::relativeQuotientScore(const Graph* G, std::set<node> A, std::set<node> S){
        float f = A.size()/G->numberOfNodes() - A.size();
        float d = 0;

        for (const auto &n: S){
            d += A.count(n) ? 1 : f;
        }

        float deltaS = 0;

        for (const auto &n: S){
            G->forNeighborsOf(n, [&](const node u, const node v){
                if (!S.count(v)){
                    deltaS += G->weight(u,v);
                }
            });
        }

        float q = d > 0 ? deltaS/d : MAXFLOAT;

        return q;
    }
}