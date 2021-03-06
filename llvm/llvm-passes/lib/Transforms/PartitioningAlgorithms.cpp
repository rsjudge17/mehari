#include "mehari/Transforms/PartitioningAlgorithms.h"

#include <math.h>	// for exp
#include <ctime> 	// for using srand with time 
#include <algorithm>
#include <limits>

// DEBUG
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"

#include "mehari/utils/ContainerUtils.h"


AbstractPartitioningMethod::~AbstractPartitioningMethod() {}


// -----------------------------------
// Random Partitioning
// -----------------------------------

unsigned int RandomPartitioning::apply(PartitioningGraph &pGraph, std::vector<std::string> &targetDevices) {
	srand(42);
	unsigned int partitionCount = targetDevices.size();
	PartitioningGraph::VertexIterator vIt = pGraph.getFirstIterator(); 
	PartitioningGraph::VertexIterator vEnd = pGraph.getEndIterator();
	for (; vIt != vEnd; ++vIt) {
		PartitioningGraph::VertexDescriptor vd = *vIt;
		pGraph.setPartition(vd, rand()%partitionCount);
	}
	return partitionCount;
}

void RandomPartitioning::balancedBiPartitioning(PartitioningGraph &pGraph) {
	srand(42);
	std::vector<unsigned int> vList;
	unsigned int vertexCount = pGraph.getVertexCount();
	for (unsigned int i=0; i<vertexCount; i++)
		vList.push_back(i);
	std::random_shuffle(vList.begin(), vList.end());
	for (unsigned int i=0; i<vertexCount; i++) {
		if (i < vertexCount/2)
			pGraph.setPartition(vList[i], 0);
		else
			pGraph.setPartition(vList[i], 1);
	}
}


// -----------------------------------
// Hierarchical Clustering
// -----------------------------------

unsigned int HierarchicalClustering::apply(PartitioningGraph &pGraph, std::vector<std::string> &targetDevices) {
	bool alwaysUseMaxPartitions = contains(targetDevices, std::string("xc7z020-1"));

	errs() << "Starting HierarchicalClustering...\n";

	partitioningGraph = pGraph;
	devices = targetDevices;
	unsigned int partitionCountMax = targetDevices.size();
	// first create a vertex in clustering graph for each of the functional units in the partitioning graph
	PartitioningGraph::VertexIterator pvIt;
	PartitioningGraph::VertexIterator pvBegin = pGraph.getFirstIterator(); 
	PartitioningGraph::VertexIterator pvEnd = pGraph.getEndIterator();
	for (pvIt = pvBegin; pvIt != pvEnd; ++pvIt) {
		PartitioningGraph::VertexDescriptor pvd = *pvIt;
		FunctionalUnitList fuList;
		fuList.push_back(pvd);
		VertexDescriptor newVertex = boost::add_vertex(clusteringGraph);
		clusteringGraph[newVertex].funcUnits = fuList;
	}

	errs() << "Created " << boost::num_vertices(clusteringGraph) << " vertices in clustering graph\n";

	// second add edges between all of the vertices of the clustering graph and set initial closeness
	VertexIterator vIt1, vIt2, vBegin, vEnd;
	boost::tie(vBegin, vEnd) = boost::vertices(clusteringGraph);
	for (vIt1 = vBegin; vIt1 != vEnd; ++vIt1) {
		VertexDescriptor vd1 = *vIt1;
		for (vIt2 = vIt1; vIt2 != vEnd; ++vIt2) {
			if (vIt2 == vIt1)
				continue; // we do not want edge from a vertex to itself
			VertexDescriptor vd2 = *vIt2;
			EdgeDescriptor ed;
			bool inserted;
			boost::tie(ed, inserted) = boost::add_edge(vd1, vd2, clusteringGraph);
			if (!initCloseness(vd1, vd2, ed))
				boost::remove_edge(ed, clusteringGraph);
		}
	}

	errs() << "Created " << boost::num_edges(clusteringGraph) << " edges in (sparse) clustering graph\n";

	// finally perform clustering and save all partitioning results that does not exceed the partitioning count
	std::vector<PartitioningGraph> partitioningResults;
	clock_t start = std::clock();
	unsigned int iteration = 0;
	while (boost::num_vertices(clusteringGraph) > (alwaysUseMaxPartitions ? partitionCountMax : 1)) {
		VertexDescriptor vd1, vd2, vdnew;
		// find best pair of vertices given by closeness
		boost::tie(vd1, vd2) = getPairMaxCloseness();
		// merge this pair to one new vertex
		vdnew = mergeVertices(vd1, vd2);
		// update the edges to connect the new vertex and remove the outdated vertices
		updateEdges(vd1, vd2, vdnew);
		removeOutdatedVertices(vd1, vd2);
		// save the current partitioning result if it does not exceed the partitioning count
		if (boost::num_vertices(clusteringGraph) <= partitionCountMax) {
			PartitioningGraph tmpPartitioningResult = partitioningGraph;
			applyClusteringOnPartitioningGraph(tmpPartitioningResult);
			partitioningResults.push_back(tmpPartitioningResult);
		}

		//TODO only print, if sufficient time has passed
		clock_t current = std::clock();
		iteration++;
		errs() << "End of iteration " << iteration << " after "
			<< format("%4.4f", (current-start) / (double)CLOCKS_PER_SEC * 1000) << " ms\n";
	}

	// determine the result with the minimum cost and save it in the original partitioning graph
	// also set the new partition count
	unsigned int newPartitionCount;
	if (!alwaysUseMaxPartitions)
		boost::tie(pGraph, newPartitionCount) = getFinalResult(partitioningResults, partitionCountMax);
	else {
		newPartitionCount = partitionCountMax;
		pGraph = partitioningResults.back();
	}


	return newPartitionCount;
}


// overwrite greater operator for graph edges to customize it
bool HierarchicalClustering::Edge::operator> (const Edge &e) const {
	if (closeness != e.closeness)
		// the closeness values are not equal, so we can choose the greater one
		return closeness > e.closeness;
	else
		// NOTE: a lower partition size product results in a higher closeness 
		return (pSizeProduct < e.pSizeProduct);
}


boost::tuple<float, unsigned int> HierarchicalClustering::closenessFunction(VertexDescriptor vd1, VertexDescriptor vd2, EdgeDescriptor ed) {
	// sum of communication costs is weighted with the product of the number of nodes that theoretically can be connected
	unsigned int comCost = clusteringGraph[ed].comCostSum;

	FunctionalUnitList funits1 = clusteringGraph[vd1].funcUnits;
	FunctionalUnitList funits2 = clusteringGraph[vd2].funcUnits;

	unsigned int pSizeProduct = funits1.size() * funits2.size();
	float closeness = float(comCost) / float(pSizeProduct);

	return boost::make_tuple(closeness, pSizeProduct);
}


bool HierarchicalClustering::initCloseness(VertexDescriptor vd1, VertexDescriptor vd2, EdgeDescriptor ed) {
	unsigned int comCost = 0;

	FunctionalUnitList funits1 = clusteringGraph[vd1].funcUnits;
	FunctionalUnitList funits2 = clusteringGraph[vd2].funcUnits;
	for (FunctionalUnitList::iterator it1 = funits1.begin(); it1 != funits1.end(); ++it1)
		for (FunctionalUnitList::iterator it2 = funits2.begin(); it2 != funits2.end(); ++it2)
			comCost += partitioningGraph.getDeviceIndependentCommunicationCost(*it1, *it2);

	clusteringGraph[ed].comCostSum = comCost;
	boost::tie(clusteringGraph[ed].closeness, clusteringGraph[ed].pSizeProduct) = closenessFunction(vd1, vd2, ed);

	return (comCost > 0);
}


void HierarchicalClustering::mergeCloseness(EdgeDescriptor ed1, EdgeDescriptor ed2, EdgeDescriptor edNew) {
	clusteringGraph[edNew].comCostSum = clusteringGraph[ed1].comCostSum + clusteringGraph[ed2].comCostSum;
	boost::tie(clusteringGraph[edNew].closeness, clusteringGraph[edNew].pSizeProduct) = 
		closenessFunction(boost::source(edNew, clusteringGraph), boost::target(edNew, clusteringGraph), edNew);
}


boost::tuple<HierarchicalClustering::VertexDescriptor, HierarchicalClustering::VertexDescriptor> 
HierarchicalClustering::getPairMaxCloseness(void) {
	if (boost::num_edges(clusteringGraph) == 0) {
		VertexIterator vIt1, vIt2, vBegin, vEnd;
		boost::tie(vBegin, vEnd) = boost::vertices(clusteringGraph);
		for (vIt1 = vBegin; vIt1 != vEnd; ++vIt1) {
			VertexDescriptor vd1 = *vIt1;
			for (vIt2 = vIt1; vIt2 != vEnd; ++vIt2) {
				if (vIt2 == vIt1)
					continue; // we do not want edge from a vertex to itself
				VertexDescriptor vd2 = *vIt2;
				EdgeDescriptor ed;
				bool inserted;
				boost::tie(ed, inserted) = boost::add_edge(vd1, vd2, clusteringGraph);
				initCloseness(vd1, vd2, ed);
			}
		}
	}
	// find edge with maximum weight (closeness value)
	EdgeIterator edgeIt, edgeEnd;
	boost::tie(edgeIt, edgeEnd) = boost::edges(clusteringGraph);
	EdgeIterator edgeMaxIt = edgeIt;
	for (; edgeIt != edgeEnd; ++edgeIt) {
		if (clusteringGraph[*edgeIt] > clusteringGraph[*edgeMaxIt])
			edgeMaxIt = edgeIt;
	}
	// get vertices that are connected by this edge
	VertexDescriptor vd1 = boost::source(*edgeMaxIt, clusteringGraph);
	VertexDescriptor vd2 = boost::target(*edgeMaxIt, clusteringGraph);

	return boost::make_tuple(vd1, vd2);
}


HierarchicalClustering::VertexDescriptor HierarchicalClustering::mergeVertices(VertexDescriptor vd1, VertexDescriptor vd2) {
	// merge lists of functional units
	FunctionalUnitList l1, l2, lnew;
	l1 = clusteringGraph[vd1].funcUnits;
	l2 = clusteringGraph[vd2].funcUnits;
	lnew.insert(lnew.end(), l1.begin(), l1.end());
	lnew.insert(lnew.end(), l2.begin(), l2.end());
	// create new vertex and return it's descriptor
	VertexDescriptor newVertex = boost::add_vertex(clusteringGraph);
	clusteringGraph[newVertex].funcUnits = lnew;
	return newVertex;
}


void HierarchicalClustering::updateEdges(VertexDescriptor vd1, VertexDescriptor vd2, VertexDescriptor vdnew) {
	VertexIterator vIt, vEnd;
	boost::tie(vIt, vEnd) = boost::vertices(clusteringGraph);
	for (; vIt != vEnd; ++vIt) {
		VertexDescriptor vd = *vIt;
		if (vd == vd1 || vd == vd2 || vd == vdnew)
			continue;
		// create new edge between current vertex and new vertex, merge closeness
		bool exists1, exists2;
		EdgeDescriptor ed1, ed2;
		boost::tie(ed1, exists1) = boost::edge(vd, vd1, clusteringGraph);
		boost::tie(ed2, exists2) = boost::edge(vd, vd2, clusteringGraph);
		if (!exists1 && !exists2)
			continue;

		EdgeDescriptor ed;
		bool inserted;
		boost::tie(ed, inserted) = boost::add_edge(vd, vdnew, clusteringGraph);
		if (exists1 && exists2)
			mergeCloseness(ed1, ed2, ed);
		else if (exists1) {
			clusteringGraph[ed].comCostSum = clusteringGraph[ed1].comCostSum;
			boost::tie(clusteringGraph[ed].closeness, clusteringGraph[ed].pSizeProduct) = 
				closenessFunction(boost::source(ed, clusteringGraph), boost::target(ed, clusteringGraph), ed);
		}
		else if (exists2) {
			clusteringGraph[ed].comCostSum = clusteringGraph[ed2].comCostSum;
			boost::tie(clusteringGraph[ed].closeness, clusteringGraph[ed].pSizeProduct) = 
				closenessFunction(boost::source(ed, clusteringGraph), boost::target(ed, clusteringGraph), ed);
		}
	}
}


void HierarchicalClustering::removeOutdatedVertices(VertexDescriptor vd1, VertexDescriptor vd2) {
	boost::clear_vertex(vd1, clusteringGraph);
	boost::clear_vertex(vd2, clusteringGraph);
	boost::remove_vertex(vd1, clusteringGraph);
	boost::remove_vertex(vd2, clusteringGraph);
}


void HierarchicalClustering::applyClusteringOnPartitioningGraph(PartitioningGraph &pGraph) {
	// save result by setting partitions
	VertexIterator vtIt, vtEnd;
	boost::tie(vtIt, vtEnd) = boost::vertices(clusteringGraph);
	for (int i=0; vtIt != vtEnd; ++vtIt, i++) {
		FunctionalUnitList flst = clusteringGraph[*vtIt].funcUnits;
		for (FunctionalUnitList::iterator fIt = flst.begin(); fIt != flst.end(); ++fIt)
			pGraph.setPartition(*fIt, i);
	}
}


boost::tuple<PartitioningGraph, unsigned int> HierarchicalClustering::getFinalResult(
		std::vector<PartitioningGraph> &partitioningResults, unsigned int maxPartitionCount) {
	// determine partitioning result with minimum cost
	std::vector<PartitioningGraph>::iterator pGraphIt = partitioningResults.begin();
	std::vector<PartitioningGraph>::iterator finalGraphIt = pGraphIt;
	for ( ; pGraphIt != partitioningResults.end(); ++pGraphIt)
		if (pGraphIt->getCriticalPathCost(devices) <= finalGraphIt->getCriticalPathCost(devices))
			finalGraphIt = pGraphIt;

	// calculate new partition count
	unsigned int newPartitionCount = maxPartitionCount - (finalGraphIt - partitioningResults.begin());

	return boost::make_tuple(*finalGraphIt, newPartitionCount);
}


void HierarchicalClustering::printGraph(void) {
	errs() << "Clustering Graph: \n";
	errs() << "\nVERTICES:\n";
	VertexIterator vertexIt, vertexEnd;
	for (boost::tie(vertexIt, vertexEnd) = boost::vertices(clusteringGraph); vertexIt != vertexEnd; ++vertexIt) { 
		errs() << "vertex " << *vertexIt << ": [ ";
		FunctionalUnitList flst = clusteringGraph[*vertexIt].funcUnits;
		for (FunctionalUnitList::iterator it = flst.begin(); it != flst.end(); ++it)
			errs() << partitioningGraph.getName(*it) << " ";
		errs() << "]\n";
	}
	errs() << "\nEDGES:\n";
	EdgeIterator edgeIt, edgeEnd;
	for (boost::tie(edgeIt, edgeEnd) = boost::edges(clusteringGraph); edgeIt != edgeEnd; ++edgeIt) {
		VertexDescriptor u = boost::source(*edgeIt, clusteringGraph), v = boost::target(*edgeIt, clusteringGraph);
		errs() << u << " -- " << v << " (closeness: " << clusteringGraph[*edgeIt].closeness << ")\n";
	}
}


// -----------------------------------
// Simulated Annealing
// -----------------------------------

unsigned int SimulatedAnnealing::apply(PartitioningGraph &pGraph, std::vector<std::string> &targetDevices) {
	// save devices used for partitioning
	devices = targetDevices;
	partitionCount = targetDevices.size();

	// configure algorithm
	Tinit = 1.0;
	Tmin = 0.1;
	iterationMax = std::max((unsigned)1000, 2*pGraph.getVertexCount()*partitionCount);
	iterationMax = std::min(iterationMax, 2000);
	tempAcceptenceMultiplicator = 3.0;
	tempDecreasingFactor = 0.95;

	// run simulated annealing algorithm
	simulatedAnnealing(pGraph, Tinit);

	return partitionCount;
}


void SimulatedAnnealing::simulatedAnnealing(State &state, Temperature initialTemperature) {
	State S = state;
	State Sbest = state;
	Temperature T = initialTemperature;
	
	srand(time(0));

	while (!frozen(T)) {
		int itCount = 0;
		int currentCost = costFunction(S);
		while (!equilibrium(itCount)) {
			State newS = S;
			randomMove(newS);
			int newCost = costFunction(newS);
			int deltaCost = newCost - currentCost;
			float deltaCostNorm = normalizeCostDifference(deltaCost, tempAcceptenceMultiplicator);
			if (acceptNewState(deltaCostNorm, T) > randomNumber()) {
				S = newS;
				currentCost = newCost;
			}
			itCount++;
		}
		T = decreaseTemperature(T);
		if (costFunction(S) - costFunction(Sbest) < 0)
			// the current result has lower cost then the best result known so far -> replace best result
			Sbest = S;
	}
	state = Sbest;
}


bool SimulatedAnnealing::frozen(Temperature temp) {
	return temp <= Tmin; // TODO: or no improvement
}


bool SimulatedAnnealing::equilibrium(int iterationCount) {
	return iterationCount >= iterationMax; // TODO: or no improvement
}


float SimulatedAnnealing::acceptNewState(int deltaCost, Temperature T) {
	return std::min(1.0, exp((-1)*float(deltaCost)/(tempAcceptenceMultiplicator*T)));
}


SimulatedAnnealing::Temperature SimulatedAnnealing::decreaseTemperature(Temperature T) {
	return tempDecreasingFactor * T;
}


int SimulatedAnnealing::costFunction(State &state) {
	// NOTE: the state is stored in a PartitioningGraph
	return state.getCriticalPathCost(devices);
}


float SimulatedAnnealing::normalizeCostDifference(int deltaCost, unsigned int K) {
	// the range of suitable cost differences depends on the parameters of the
	// (exponential) acceptance function, especially the multiplication factor for the temperature
	// accept = e^(- deltaCost / K * T)
	// to approximate the cost range depending on K the linear function f = 2*k+1 is used
	unsigned int rangeMax = 2*K+1;
	return (rangeMax / (K*1000.0) * deltaCost);
}


void SimulatedAnnealing::randomMove(State &state) {
	// NOTE: the state is stored in a PartitioningGraph
	PartitioningGraph::VertexDescriptor vd = state.getRandomVertex();
	unsigned int oldPartition = state.getPartition(vd);
	unsigned int newPartition;
	do {
		newPartition = rand() % partitionCount;
	} while(newPartition == oldPartition);
	state.setPartition(vd, newPartition);
}


double SimulatedAnnealing::randomNumber(void) {
	return (rand() / double(RAND_MAX));
}



// -----------------------------------
// Kernighan Lin
// -----------------------------------

unsigned int KernighanLin::apply(PartitioningGraph &pGraph, std::vector<std::string> &targetDevices) {
	// save devices used for partitioning
	devices = targetDevices;
	unsigned int partitionCount = targetDevices.size();

	// NOTE: currently this algorithm is only implemented for bi-partitioning
	if (partitionCount != 2) {
		errs() << "WARNING: Kernighan Lin currently only can handle bi-partitioning tasks!\n";
		return partitionCount;
	}

	PartitioningGraph currentResult = pGraph;

	// create balanced initial state
	// RandomPartitioning P;
	// P.balancedBiPartitioning(currentResult);

	// perform iteration until there is no improvement
	bool improved = false;
	do {
		// set iteration count to minimum of the number of vertices in one of the two partitions
		unsigned int iterationCount = std::min(currentResult.getVertexCountForPartition(0), 
			currentResult.getVertexCountForPartition(1));

		// create the additional information mapping for all vertices of the partitioning graph
		additionalVertexInformation.clear();
		for (unsigned int i=0; i<currentResult.getVertexCount(); i++) {
			additionalVertexInformation.push_back(AdditionalVertexInfo());
		}
		// initialize the costDifference values for all vertices
		createInitialCostDifferences(currentResult);
		// save some information for each iteration
		std::vector<boost::tuple<unsigned int, unsigned int> > interchanges;
		int currentTotalGain = 0, maxTotalGain = std::numeric_limits<int>::min(), maxTotalGainIndex = 0;
		// iterate over the vertices and interchange them
		for (unsigned int i=0; i<iterationCount; i++) {
			// find pair that should be interchanged
			unsigned int v1, v2;
			int gain;
			boost::tie(v1, v2, gain) = findInterchangePair(currentResult);
			// save the pair in the interchange list and check if it's the best result
			interchanges.push_back(boost::make_tuple(v1, v2));
			currentTotalGain += gain;
			if(currentTotalGain > maxTotalGain) {
				maxTotalGain = currentTotalGain;
				maxTotalGainIndex = i;
			}
			// remove the two vertices from further consideration in this pass
			lockMovedVertices(v1, v2);
			// update costDifference values for the remaining vertices
			updateCostDifferences(v1, v2, currentResult);
		}
		// unlock all vertices to enable them for interchanging during the next iteration
		unlockAllVertices();
		if ((improved = (maxTotalGain > 0)))
			// save current result
			applyInterchanges(interchanges, maxTotalGainIndex, currentResult);
	} while (improved);

	// return last improved result
	pGraph = currentResult;
	return partitionCount;
}


void KernighanLin::createInitialCostDifferences(PartitioningGraph &pGraph) {
	std::vector<AdditionalVertexInfo>::iterator it;
	for(it = additionalVertexInformation.begin(); it != additionalVertexInformation.end(); ++it) {
		unsigned int internalCosts, externalCosts;
		boost::tie(internalCosts, externalCosts) = pGraph.getInternalExternalCommunicationCost(
			(it-additionalVertexInformation.begin()));
		it->costDifference = externalCosts - internalCosts;
	}
}


void KernighanLin::updateCostDifferences(unsigned int icV1, unsigned int icV2, PartitioningGraph &pGraph) {
	std::vector<AdditionalVertexInfo>::iterator it;
	for(it = additionalVertexInformation.begin(); it != additionalVertexInformation.end(); ++it) {
		if (it->locked)
			continue;
		unsigned int v = (it-additionalVertexInformation.begin());
		unsigned int cost1 = pGraph.getDeviceIndependentCommunicationCost(v, icV1);
		unsigned int cost2 = pGraph.getDeviceIndependentCommunicationCost(v, icV2);
		if (cost1 > 0 || cost2 > 0) {
			if (pGraph.getPartition(v) == pGraph.getPartition(icV1)) {
				it->costDifference += 2*cost1 - 2*cost2;
			}
			else {
				it->costDifference += 2*cost2 - 2*cost1;
			}
		}
	}
}


boost::tuple<unsigned int, unsigned int, unsigned int> 
KernighanLin::findInterchangePair(PartitioningGraph &pGraph) {
	unsigned int v1, v2;
	int costReduction = std::numeric_limits<int>::min();
	std::vector<AdditionalVertexInfo>::iterator it1, it2;
	for(it1 = additionalVertexInformation.begin(); it1 != additionalVertexInformation.end(); ++it1) {
		if (it1->locked)
			continue;
		for(it2 = it1; it2 != additionalVertexInformation.end(); ++it2) {
			if (it2->locked || it1 == it2)
				continue;
			unsigned int vNr1 = (it1-additionalVertexInformation.begin());
			unsigned int vNr2 = (it2-additionalVertexInformation.begin());
			if (pGraph.getPartition(vNr1) != pGraph.getPartition(vNr2)) {
				// the current pair of nodes has different partitions -> check if we should interchange the nodes
				int newCostReduction = it1->costDifference + it2->costDifference 
					- 2*pGraph.getCommunicationCost(vNr1, vNr2, 
						devices[pGraph.getPartition(vNr1)], devices[pGraph.getPartition(vNr2)]);
				if (newCostReduction > costReduction) {
					// the current pair of nodes would result in a higher cost reduction if we interchange them
					// -> save the current pair and set new cost reduction value
					v1 = vNr1; v2 = vNr2;
					costReduction = newCostReduction;
				}
			}
		}
	}
	return boost::make_tuple(v1, v2, costReduction);
}


void KernighanLin::lockMovedVertices(unsigned int v1, unsigned int v2) {
	additionalVertexInformation[v1].locked = true;
	additionalVertexInformation[v2].locked = true;
}


void KernighanLin::unlockAllVertices(void) {
	std::vector<AdditionalVertexInfo>::iterator it;
	for(it = additionalVertexInformation.begin(); it != additionalVertexInformation.end(); ++it)
		it->locked = false;
}


void KernighanLin::applyInterchanges(std::vector<boost::tuple<unsigned int, unsigned int> > &interchanges, 
	unsigned int maxTotalGainIndex, PartitioningGraph &result) {
	for (unsigned int i=0; i<=maxTotalGainIndex; i++) {
		unsigned int v1, v2;
		boost::tie(v1, v2) = interchanges[i];
		unsigned int tmpPartition = result.getPartition(v1);
		result.setPartition(v1, result.getPartition(v2));
		result.setPartition(v2, tmpPartition);
	}
}
