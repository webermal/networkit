//
// Created by malte on 26.02.21.
//

#include "networkit/community/ImproveClustering.hpp"
#include <networkit/graph/Graph.hpp>
#include "networkit/community/MinCutStoerWagner.hpp"
#include <iostream>


namespace NetworKit {

    ImproveClustering::ImproveClustering(const Graph &G, const Partition initPartition):
      CommunityDetectionAlgorithm(G), initPartition(std::move(initPartition)), flowGraph(G) {

        s = flowGraph.addNode();
        t = flowGraph.addNode();
        // TODO check if initPartition has exactely two Groups
        std::map<index, count> initPartitionSizes = initPartition.subsetSizeMap();
        minS = std::min_element(
                         initPartitionSizes.begin(),
                         initPartitionSizes.end(),
                         [](const auto &l, const auto &r){ return l.second < r.second;}
                         )->first;

        initPartitionSet1 = initPartition.getMembers(minS);
        initPartitionSet1.insert(s);
        f_a = (double)initPartitionSet1.size()/(double)(G.numberOfNodes() - initPartitionSet1.size());

        G.forNodes([&](const node n){
            if (initPartition.subsetOf(n) == minS){
                flowGraph.addEdge(s, n, 0);
            } else {
                flowGraph.addEdge(n, t, 0);
            }
        });
    }
    //ImproveClustering::ImproveClustering(const Graph &G, const Partition &initPartition): G(&G), initPartition(initPartition){}

    void ImproveClustering::run() {

        result = initPartition;

        updateEdgeWeights(flowGraph, relativeQuotientScore(G, initPartition.getMembers(minS), initPartition.getMembers(minS)));

        double alpha_0 = relativeQuotientScore(&flowGraph, initPartitionSet1,initPartitionSet1);
        double alpha;
        int i = 0;
        do {
            i += 1;
            std::cout << i << "\n";
            alpha = alpha_0;

            //MinCutStoerWagner minCut(flowGraph);
            //minCut.run();
            Partition minCutPartition = initPartition;//minCut.getPartition();

            std::map<index, count> minCutSizes = minCutPartition.subsetSizeMap();
            index minS = std::min_element(
                             minCutSizes.begin(),
                             minCutSizes.end(),
                             [](const auto &l, const auto &r){ return l.second < r.second;}
                             )->first;

            /*alpha_0 = relativeQuotientScore(
                G,
                initPartitionSet1,
                minCutPartition.getMembers(minCutPartition.subsetOf(s))
                );
*/
            result = minCutPartition;

        } while (alpha_0 <= alpha and i < 3);

        //result.remove(s);
        //result.remove(t);

        hasRun = true;
    }

    double ImproveClustering::relativeQuotientScore(const Graph* G, std::set<node> A, std::set<node> S){
        //double f = (double)A.size() / (double )(G->numberOfNodes() - A.size());
        double d = 0.0;

        for (const auto &n: S){
            d += A.count(n) ? 1.0 : f_a;
        }

        double deltaS = 0.0;

        for (const auto &n: S){
            G->forNeighborsOf(n, [&](const node u, const node v){
                if (!S.count(v)){
                    deltaS += G->weight(u,v);
                }
            });
        }

        double q = d > 0.0 ? deltaS/d : std::numeric_limits<double>::max();

        return q;
    }

    void ImproveClustering::updateEdgeWeights(Graph G, double alpha){
        G.forNeighborsOf(s, [&](node n){
            G.setWeight(s, n, alpha);
        });

        G.forNeighborsOf(t, [&](node n){
            G.setWeight(n, t, alpha*f_a);
        });
    }
}