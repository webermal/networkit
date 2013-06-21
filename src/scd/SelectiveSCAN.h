/*
 * SelectiveSCAN.h
 *
 *  Created on: 14.06.2013
 *      Author: cls
 */

#ifndef SELECTIVESCAN_H_
#define SELECTIVESCAN_H_

#include "SelectiveCommunityDetector.h"

namespace NetworKit {

class SelectiveSCAN: public NetworKit::SelectiveCommunityDetector {

public:
	double epsilon;
	double mu;

public:

	SelectiveSCAN(Graph& G);

	virtual ~SelectiveSCAN();

	virtual std::unordered_map<node, std::unordered_set<node> > run(std::unordered_set<node> seeds);

protected:

	virtual double nodeDistance(node u, node v);



	virtual void expandCore(node core, node label, std::unordered_set<node>* community,
					std::unordered_map<node, node>* nodesState, std::vector<node>* candidates);

	virtual std::pair<bool,std::vector<node>> isCore(node u);

};

} /* namespace NetworKit */
#endif /* SELECTIVESCAN_H_ */
