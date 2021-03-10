//
// Created by malte on 26.02.21.
//

#include <networkit/community/ImproveClustering.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/community/MinCutStoerWagner.hpp>
#include <iostream>


namespace NetworKit {

    ImproveClustering::ImproveClustering(const Graph &G, const Partition initPartition):
      CommunityDetectionAlgorithm(G), iPartition(initPartition.getVector()), flowGraph(G) {

        // add source (s) and sink (t) to the flowGraph
        s = flowGraph.addNode();
        t = flowGraph.addNode();

        // TODO check if initPartition has exactely two Groups
        iPartition.compact();
        minS = iPartition.getMembers(0).size() < iPartition.getMembers(1).size() ? 0 : 1;

        f_a = (double)iPartition.getMembers(minS).size()/
              (double)(G.numberOfNodes() - iPartition.getMembers(minS).size());

        G.forNodes([&](const node n){
            if (iPartition.subsetOf(n) == minS){
                flowGraph.addEdge(s, n, 1);
            } else {
                flowGraph.addEdge(n, t, 1);
            }
        });
    }

    void ImproveClustering::run() {

        INFO("FLOW GRAPH deg s", flowGraph.degree(s));
        INFO("FLOW GRAPH deg t", flowGraph.degree(t));

        INFO("Init Partition", iPartition.getVector());
        double alpha_0 = relativeQuotientScore(G, iPartition.getMembers(minS), iPartition.getMembers(minS));
        double alpha;
        int i = 0;
        do {
            i += 1;
            std::cout << i << "\n";

            updateEdgeWeights(alpha_0);

            alpha = alpha_0;

            MinCutStoerWagner minCut(flowGraph);
            minCut.run();
            Partition minCutPartition = minCut.getPartition();


            INFO("MIN CUT IN IMPROVE RUN: ", minCutPartition.getVector());

            std::set<index> SetS(minCutPartition.getMembers(minCutPartition.subsetOf(s)));
            SetS.erase(s);
            alpha_0 = relativeQuotientScore(
                G,
                iPartition.getMembers(minS),
                SetS
                );
            INFO("ALPHA 0: ", alpha_0);
            // Computing result partition
            // TODO Do this better
            std::vector<index> resultVec;
            // TODO Problem if the node are not enumerated from 0 to one
            resultVec.resize(flowGraph.upperNodeIdBound());
            minCutPartition.compact();

            std::set<index> sourceSet(minCutPartition.getMembers(minCutPartition.subsetOf(s)));
            std::set<index> sinkSet(minCutPartition.getMembers(minCutPartition.subsetOf(t)));

            sourceSet.erase(s);
            sinkSet.erase(t);
            resultVec[s] = 2;
            resultVec[t] = 3;
            for (const index &i: sourceSet){
                resultVec[i] = 0;
            }
            for (const index &i: sinkSet){
                resultVec[i] = 1;
            }
            result = Partition(resultVec);

        } while (i < 10);

        hasRun = true;
    }

    double ImproveClustering::relativeQuotientScore(const Graph* G, std::set<node> A, std::set<node> S){
        //double f = (double)A.size() / (double )(G->numberOfNodes() - A.size());
        double d = 0.0;

        // TODO not so ugly!!!!

        INFO("SET A", A);
        INFO("SET S", S);
        for (const node n: S){
            if (n != s and n != t){
                d += A.find(n) != A.end() ? 1.0 : -f_a;
                INFO("DDDDDD: ",d );
            }

        }
        INFO("d hat den Wert: ", d);

        double deltaS = 0.0;
        // TODO use Edgcut get Quality
        flowGraph.forEdges([&](const node u, const node v, const edgeweight w){
            if (u != t and v != t and u != s and v != s){
                if ((S.count(u) and !S.count(v)) or (!S.count(u) and S.count(v))){
                    INFO("NODE ", v, " TO NODE ", u, " WITH WEIGHT ",  w);
                    deltaS += w;
                }
            }
        });

        flowGraph.forEdges([&](const node u, const node v, const edgeweight w){
            INFO(" EDGE: ", u, " TO " ,v , " WITH WEIGHT ", w );
        });

        INFO("DELTA S: ", deltaS);
        INFO("D: ", d);
        double q = d > 0.0 ? deltaS/d : std::numeric_limits<double>::max();

        return q;
    }

    void ImproveClustering::updateEdgeWeights(double alpha){
        flowGraph.forNeighborsOf(s, [&](node n){
            flowGraph.setWeight(s, n, alpha);
            INFO("ALPHA: ", alpha);
        });

        flowGraph.forNeighborsOf(t, [&](node n){
            flowGraph.setWeight(n, t, alpha*f_a);
            INFO("ALPHA*FA: ", alpha*f_a);
        });
    }
}