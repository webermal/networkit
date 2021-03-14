//
// Created by malte on 26.02.21.
//

#include <networkit/community/ImproveClustering.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/community/MinCutStoerWagner.hpp>
#include <iostream>
#include <networkit/community/GraphClusteringTools.hpp>
#include <networkit/community/EdgeCut.hpp>
#include <networkit/flow/EdmondsKarp.hpp>

namespace NetworKit {

    ImproveClustering::ImproveClustering(const Graph &G, const Partition initPartition, const count maxIter):
      CommunityDetectionAlgorithm(G, initPartition), flowGraph(G, true, true), isDirected(G.isDirected()), maxIter(maxIter) {

        // add source (s) and sink (t) to the flowGraph
        s = flowGraph.addNode();
        t = flowGraph.addNode();

        // Check if initial Partition has 2 Clusters
        if(result.subsetSizes().size() != 2){
            throw std::runtime_error("The number of Clusters in the initial Partition must be 2");
        }

        //Check if initial Partition is a proper Clustering
        if(!GraphClusteringTools::isProperClustering(G, result)){
            throw std::runtime_error("The initial Partition must be a proper clustering");
        }

        // Compute the set of indices A from the initial Partition
        result.compact();
        index minS = result.getMembers(0).size() < result.getMembers(1).size() ? 0 : 1;
        A = result.getMembers(minS);

        // Compute f_a the ration between A and the complement of A
        f_a = (double)A.size()/
              (double)(G.numberOfNodes() - A.size());

        // Add edges to the transformed Graph "flowGraph" from s to A and from complement(A) to t
        G.forNodes([&](const node n){
            if (A.find(n) != A.end()){
                flowGraph.addEdge(s, n, 1);
            } else {
                flowGraph.addEdge(n, t, 1);
            }
        });

        // index Edges for Edmonds Karp algorithm
        flowGraph.indexEdges();
    }

    void ImproveClustering::run() {

        INFO("FLOW GRAPH deg s", flowGraph.degree(s));
        INFO("FLOW GRAPH deg t", flowGraph.degree(t));

        INFO("Init Partition", result.getVector());
        EdgeCut ec;
        double boundaryA = ec.getQuality(result, *G) + A.size();
        double alpha_0 = boundaryA/std::min(A.size(), (G->numberOfNodes() - A.size()));

        double alpha;
        iter = 0;

        do {
            // counter for
            iter += 1;
            // std::cout << i << "\n";

            updateEdgeWeights(alpha_0);

            alpha = alpha_0;

            EdmondsKarp edmondsKarp(flowGraph, s, t);
            edmondsKarp.run();

            Partition minCutPartition(flowGraph.numberOfNodes(), 0);

            S0 = edmondsKarp.getSourceSet();

            for (const node n: S0){
                minCutPartition[n] = 1;
            }


            /*
            MinCutStoerWagner minCut(flowGraph, s, t);
            minCut.run();
            Partition minCutPartition = minCut.getPartition();
*/
            EdgeCut ec;

            INFO("boundary S: ", ec.getQuality(minCutPartition, flowGraph));
            INFO("CUT VALUE From edmonds Karp: ", edmondsKarp.getMaxFlow());


            alpha_0 = relativeQuotientScore(minCutPartition);

            INFO("ALPHA 0: ", alpha_0);

        } while (alpha_0 > alpha and iter < 4);

        hasRun = true;
    }

    double ImproveClustering::relativeQuotientScore(const Partition& p){

        S = p.getMembers(p.subsetOf(s));

        double dAOfS = 0.0;
        for (const node n: S){
            dAOfS += A.find(n) != A.end() ? 1.0 : -f_a;
        }

        INFO("d hat den Wert: ", dAOfS);

        // Compute the Cut-Value of S / complement(S)
        //EdgeCut ec;
        //double boundaryS = ec.getQuality(p, flowGraph);

        EdmondsKarp edmondsKarp(flowGraph, s, t);
        edmondsKarp.run();
        double boundaryS = 2*edmondsKarp.getMaxFlow();

        INFO("DELTA S: ", boundaryS);
        INFO("D: ", dAOfS);
        double q = dAOfS > 0.0 ? boundaryS/dAOfS : std::numeric_limits<double>::max();

        return q;
    }



    void ImproveClustering::updateEdgeWeights(double alpha){
        flowGraph.forNeighborsOf(s, [&](node n){
            flowGraph.setWeight(s, n, alpha);
            //INFO("ALPHA: ", alpha);
        });

        flowGraph.forInNeighborsOf(t, [&](node n){
            flowGraph.setWeight(n, t, alpha*f_a);
            //INFO("ALPHA*FA: ", alpha*f_a);
        });
    }
}