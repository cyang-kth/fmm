//
// Created by Can Yang on 2020/3/22.
//

#include "mm/fmm/ubodt.hpp"
#include "util/util.hpp"

#include <fstream>
#include <stdexcept>

#ifdef BOOST_OS_WINDOWS
#include <boost/throw_exception.hpp>
#endif
#include <boost/format.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;
UBODT::UBODT(int buckets_arg, int multiplier_arg, NetworkGraph graph_arg) :
    buckets(buckets_arg), multiplier(multiplier_arg), graph(graph_arg) {
  SPDLOG_TRACE("Intialization UBODT with buckets {} multiplier {}",
               buckets, multiplier);
  hashtable = (Record **) malloc(sizeof(Record *) * buckets);
  for (int i = 0; i < buckets; i++) {
    hashtable[i] = nullptr;
  }
  SPDLOG_TRACE("Intialization UBODT finished");
}

UBODT::~UBODT() {
  /* Clean hashtable */
  SPDLOG_TRACE("Clean UBODT");
  int i;
  for (i = 0; i < buckets; ++i) {
    Record *head = hashtable[i];
    Record *curr;
    while ((curr = head) != nullptr) {
      head = head->next;
      free(curr);
    }
  }
  // Destory hash table pointer
  free(hashtable);
  SPDLOG_TRACE("Clean UBODT finished");
}

Record *UBODT::look_up(NodeIndex source, NodeIndex target) const {
  unsigned int h = cal_bucket_index(source, target);
  Record *r = hashtable[h];
  while (r != nullptr) {
    if (r->source == source && r->target == target) {
      return r;
    } else {
      r = r->next;
    }
  }
  return r;
}

Record* UBODT::look_up_or_make(NETWORK::NodeIndex source,
		NETWORK::NodeIndex target) {
	Record *r = look_up(source, target);
	// No transition exist from source to target
	if (r == nullptr) {
		Heap Q;
		PredecessorMap pmap;
		DistanceMap dmap;

		graph.shortest_path_dijkstra(&Q, &pmap, &dmap, this, source, target);

		std::stack<NodeIndex> route;

		for (auto iter = pmap.begin(); iter != pmap.end(); ++iter) {
			NodeIndex cur_node = iter->first;
			if (!Q.contain_node(cur_node) && !look_up(source, cur_node)) {
				// If node isn't in the heap, it means that pmap and dmap hold truthful information about distance
				// If there is no look_up value, it must be stored

				NodeIndex prev_n = iter->second;
				// Create a stack with the route from source to cur_node
				for(NodeIndex u=cur_node; u != source; u=pmap[u])
					route.push(u);

				// Insert ods from the former to the latter
				// The former is always the source
				NodeIndex former = source;
				while(former != cur_node && !look_up(former, cur_node)) {
					// Repeat until the route reached cur_node
					// or the rest of the route is already in the table

					r = (Record*) malloc(sizeof(Record));
					r->source = former;
					r->target = cur_node;

					// First node of this route will be the former of the next
					NodeIndex s = former;
					former = route.top();
					route.pop();
					r->first_n = former;
					r->prev_n = prev_n;
					r->next_e = graph.get_edge_index(s, former, dmap[former]-dmap[s]);
					r->cost = dmap[cur_node]-dmap[s];
					r->next = nullptr;
					insert(r);
				}
			}
		}
		r = look_up(source, target);
	}
	return r;
}

std::vector<EdgeIndex> UBODT::look_sp_path(NodeIndex source,
                                           NodeIndex target) {
  std::vector<EdgeIndex> edges;
  if (source == target) { return edges; }
  Record *r = look_up_or_make(source, target);
  // No transition exist from source to target
  if (r == nullptr) { return edges; }
  while (r->first_n != target) {
    edges.push_back(r->next_e);
    r = look_up(r->first_n, target);
  }
  edges.push_back(r->next_e);
  return edges;
}

std::vector<std::vector<NETWORK::EdgeIndex>> UBODT::look_k_sp_path(
		const NETWORK::NodeIndex source, const NETWORK::NodeIndex target, int K) {
	// Return empty list if target and source are the same
	std::vector<std::vector<NETWORK::EdgeIndex>> returnV;
	if (source == target) {return returnV;}

	std::vector<std::vector<Record*>> routes(K);
	routes.at(0) = {look_up_or_make(source, target)};

	struct RoutesNode {
		std::vector<Record*> index; /**< Index of a node in the heap */
		double value; /**< Value of a node in the heap */
		bool operator<(const RoutesNode &rhs) const {
			return value < rhs.value;
		}
	};
	FibHeap<RoutesNode> route_candidates;

	std::vector<std::unique_ptr<Record>> temporaryRecords;

	const Graph_T g = graph.get_boost_graph();

	/** Lambda functions to help code reusability inside the function **/
	auto nextNode = [this](Record* &r, std::vector<Record*>::iterator &recordIterator, const NodeIndex currentNode){
		// If at the end of an OD pair, get the next pair
		if(currentNode == r->target)
			r = *++recordIterator;

		// get route from current node to the target, and get next node
		r = look_up(currentNode, r->target);
		return r-> first_n;
	};

	auto getOutEdges = [g](NodeIndex source){
		static OutEdgeIterator out_i, out_end;
		std::map<EdgeIndex, EdgeDescriptor> out_edges;
		for(boost::tie(out_i, out_end) = boost::out_edges(source, g);
				out_i != out_end; out_i++){
			EdgeDescriptor e = *out_i;
			out_edges[g[e].index] = e;
		}
		return out_edges;
	};

	auto getMinimalPathAfterEdge = [this, g, target, &temporaryRecords](const EdgeDescriptor e, const NodeIndex previousNode, const std::vector<Record*> &basePath){
		NodeIndex nodeAfterEdge = boost::target(e, g);

		if(nodeAfterEdge == previousNode)
			throw std::invalid_argument("Loop edge");

		if(!basePath.empty() && nodeAfterEdge == basePath.back()->prev_n)
			throw std::invalid_argument("Edge goes backward");

		Record *nextOD = look_up_or_make(nodeAfterEdge, target);
		if (nextOD == nullptr)
			throw std::invalid_argument("No od pair");

		EdgeProperty eProperty = g[e];
		RoutesNode returnV;
		returnV.value = eProperty.length + nextOD -> cost;
		for(Record* record : basePath) {
			returnV.value += record->cost;
			returnV.index.push_back(record);
		}

		// push the edge as a route
		Record *edgeRecord = look_up_or_make(previousNode, nodeAfterEdge);
		// If edge is not the sp, the edge must be pushed, mocking a sp
		if(edgeRecord->next_e != eProperty.index) {
			std::unique_ptr<Record> tempRecord(new Record);
			tempRecord->source = previousNode;
			tempRecord->prev_n = previousNode;
			tempRecord->target = nodeAfterEdge;
			tempRecord->first_n = nodeAfterEdge;
			tempRecord->next_e = eProperty.index;
			tempRecord->cost = eProperty.length;
			tempRecord->next = nullptr;
		    temporaryRecords.push_back(std::move(tempRecord));
		    edgeRecord = temporaryRecords.back().get();
		}
		returnV.index.push_back(edgeRecord);
		// Push the last part of the route
		returnV.index.push_back(nextOD);
		return returnV;
	};

	auto isSubRoute = [this, &nextNode](std::vector<Record*> &route1,
			std::vector<Record*> &smallerRoute, Record* &r, const NodeIndex lastNodeBase) {
		auto iterR = route1.begin();
		r = *iterR;
		auto iterBasePath = smallerRoute.begin();
		Record *basePathRecord = *iterBasePath;

		// basePath contains all record, from
		// its source to its target. Then, advance
		while (iterBasePath + 1 != smallerRoute.end() && r == basePathRecord) {
			r = *++iterR;
			basePathRecord = *++iterBasePath;
		}

		// Walk through route1 to check if the route matches it
		NodeIndex nextNodeRoute = r->source;
		NodeIndex nextNodeBase = basePathRecord->source;
		while (nextNodeBase != lastNodeBase
				&& nextNodeBase == nextNodeRoute) { // Path still didn't get to spurNode
			nextNodeRoute = nextNode(r, iterR, nextNodeRoute);
			nextNodeBase = nextNode(basePathRecord, iterBasePath,
					nextNodeBase);
		}

		// Update r, if applicable
		if(nextNodeRoute == r->target && iterR+1 != route1.end())
			r = *++iterR;
		if(r != nullptr && nextNodeRoute != r->target)
			r = look_up(nextNodeRoute, r->target);

		return nextNodeBase == nextNodeRoute;
	};
	/** End of lambda functions **/

	int k=1;
	// Implementation of Yen's algorithm
	// https://doi.org/10.1287/mnsc.17.11.712
	for(; k<K; k++){
		std::vector<Record*> latestRoute = routes.at(k-1);

		// The spur node is the ith node of latestRoute
		auto spurNodeIter = latestRoute.begin();
		Record* spurNodeRecord = *spurNodeIter;
		NodeIndex spurNode = spurNodeRecord->source;
		std::vector<Record*> basePath;
		// Repeat until the last node
		while(spurNode != target){

			// Load all edges that come out of the spur node
			std::map<EdgeIndex, EdgeDescriptor> out_edges = getOutEdges(spurNode);

			// Eliminate edges of the spurNode used by routes
			// that use basePath integrally
			for(int i=0; i<k; i++) {
				// Special case of spurNode=source
				Record *r = routes.at(i).front();
				if(basePath.empty()){
					out_edges.erase(r->next_e);
					continue;
				}

				// Erase edge of next
				if(isSubRoute(routes.at(i), basePath, r, spurNode))
					out_edges.erase(r->next_e);
			}

			// Calculate shortest path from source to destination
			//without some edges out of spurNode
			for(auto edge: out_edges) {
				try {
					auto candidate = getMinimalPathAfterEdge(edge.second, spurNode, basePath);
					route_candidates.push(candidate);
				} catch (std::invalid_argument const& ex) {
					continue;
				}
			}

			/** Update spurnode and basepath **/
			auto formerSpurNodeIter = spurNodeIter;
			spurNode = nextNode(spurNodeRecord, spurNodeIter, spurNode);
			// Check if the sp inside latestRoute changed
			if(formerSpurNodeIter != spurNodeIter || basePath.empty())
				basePath.push_back(look_up((*spurNodeIter)->source, spurNode));
			else
				basePath.back() = look_up((*spurNodeIter)->source, spurNode);
		}

		/** Test if the candidate is valid **/
		// It will be invalid if the last od passes its spur node
		while (!route_candidates.empty()) {
			std::vector<Record*> topCandidate = route_candidates.top().index;
			route_candidates.pop();

			auto lastIter = topCandidate.end()-2;
			NodeIndex candidateSpurNode = (*lastIter++)->source;
			auto candidateIter=topCandidate.begin();
			Record *r = *candidateIter;

			// Nodes before and including spur
			std::set<NodeIndex> nodesUntilSpur;
			while(candidateIter!=lastIter) {
				nodesUntilSpur.insert(r->source);
				while(r->first_n != r->target){
					r = look_up(r->first_n, r->target);
					nodesUntilSpur.insert(r->source);
				}
				r = *++candidateIter;
			}

			while(r->first_n != target && nodesUntilSpur.find(r->source) == nodesUntilSpur.end()) {
				r = look_up(r->first_n, target);
			}

			if(nodesUntilSpur.find(r->source) == nodesUntilSpur.end()) {
				// Valid candidate
				// Check if not already included
				int i = 0;
				Record *r;
				for(; i<k; i++)
					if(isSubRoute(routes.at(i), topCandidate, r, target))
						break;

				//Route already included, trying again
				if(i==k) {
					routes.at(k) = topCandidate;
					// End route_candidates loop
					break;
				}
			} else {
				// Candidate contains some node twice.
				NodeIndex duplicatedNode = r->source;
				// Get last record, since this is the record
				// where the mistake is.
				r = topCandidate.back();
				//  The process of finding a new route consists
				// of splitting the last record in from two to three records
				// The source of the first one must be firstSource
				NodeIndex firstSource = r->source;

				while(r->source != duplicatedNode) { //Next lastSource must not be the duplicated node
					// If there are three records, the last one's source will be lastSource
					NodeIndex lastSource = r->source;

					// make route as basepath-> (firstSource,newSource) -> (newSource,target)
					// Append all route, except the last element
					std::vector<Record*> candidateBasePath(topCandidate.begin(), candidateIter);
					if(firstSource != lastSource)
						candidateBasePath.push_back(look_up(firstSource, lastSource));

					for(auto edge: getOutEdges(lastSource)) {
						if(edge.first == r->next_e)
							continue;

						try {
							auto newCandidate = getMinimalPathAfterEdge(edge.second, lastSource, candidateBasePath);
							route_candidates.push(newCandidate);
						} catch (std::invalid_argument const& ex) {
							continue;
						}
					}
					r = look_up(r->first_n, r->target);
				}
			}
		}
		if(route_candidates.empty())
			break;
	}
	for (int i=0; i<k; i++){
		std::vector<EdgeIndex> edges;
		auto iterR = routes.at(i).begin();
		Record *r = *iterR;
		NodeIndex currentNode = r->source;
		while(currentNode != target){
			currentNode = nextNode(r, iterR, currentNode);
			edges.push_back(r->next_e);
		}
		returnV.push_back(edges);
	}

	return returnV;//routes;

}

C_Path UBODT::construct_complete_path(int traj_id, const TGOpath &path,
                                      const std::vector<Edge> &edges,
                                      std::vector<int> *indices,
                                      double reverse_tolerance) {
  C_Path cpath;
  if (!indices->empty()) indices->clear();
  if (path.empty()) return cpath;
  int N = path.size();
  cpath.push_back(path[0]->c->edge->id);
  int current_idx = 0;
  indices->push_back(current_idx);
  SPDLOG_TRACE("Insert index {}", current_idx);
  for (int i = 0; i < N - 1; ++i) {
    const Candidate *a = path[i]->c;
    const Candidate *b = path[i + 1]->c;
    SPDLOG_DEBUG("Check point {} a {} b {}", i, a->edge->id, b->edge->id);
    if ((a->edge->id != b->edge->id) || (a->offset - b->offset >
        a->edge->length * reverse_tolerance)) {
      // segs stores edge index
      auto segs = look_sp_path(a->edge->target, b->edge->source);
      // No transition exist in UBODT
      if (segs.empty() && a->edge->target != b->edge->source) {
        SPDLOG_DEBUG("Edges not found connecting a b");
        SPDLOG_DEBUG("reverse movement {} tolerance {}",
          a->offset - b->offset, a->edge->length * reverse_tolerance);
        SPDLOG_WARN("Traj {} unmatched as edge {} L {} offset {}"
          " and edge {} L {} offset {} disconnected",
          traj_id, a->edge->id, a->edge->length, a->offset,
          b->edge->id, b->edge->length, b->offset);

        indices->clear();
        return C_Path();
      }
      if (segs.empty()) {
        SPDLOG_DEBUG("Edges ab are adjacent");
      } else {
        SPDLOG_DEBUG("Edges connecting ab are {}", segs);
      }
      for (int e:segs) {
        cpath.push_back(edges[e].id);
        ++current_idx;
      }
      cpath.push_back(b->edge->id);
      ++current_idx;
      indices->push_back(current_idx);
      SPDLOG_TRACE("Insert index {}", current_idx);
    } else {
      indices->push_back(current_idx);
      SPDLOG_TRACE("Insert index {}", current_idx);
    }
  }
  return cpath;
}

double UBODT::get_delta() const {
  return delta;
}

unsigned int UBODT::cal_bucket_index(NodeIndex source, NodeIndex target) const {
  return (source * multiplier + target) % buckets;
}


void UBODT::insert(Record *r) {
  //int h = (r->source*multiplier+r->target)%buckets ;
  int h = cal_bucket_index(r->source, r->target);
  r->next = hashtable[h];
  hashtable[h] = r;
  if (r->cost > delta) delta = r->cost;
  ++num_rows;
}

long UBODT::estimate_ubodt_rows(const std::string &filename) {
  struct stat stat_buf;
  long rc = stat(filename.c_str(), &stat_buf);
  if (rc == 0) {
    int file_bytes = stat_buf.st_size;
    SPDLOG_TRACE("UBODT file size is {} bytes", file_bytes);
    std::string fn_extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(fn_extension.begin(),
                   fn_extension.end(),
                   fn_extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (fn_extension == "csv" || fn_extension == "txt") {
      int row_size = 36;
      return file_bytes / row_size;
    } else if (fn_extension == "bin" || fn_extension == "binary") {
      Record r;
      // When exporting to a file using boost binary writer,
      // the padding is removed.
      int row_size = 28;
      return file_bytes / row_size;
    }
  }
  return -1;
}

int UBODT::find_prime_number(double value) {
  std::vector<int> prime_numbers = {
      5003, 10039, 20029, 50047, 100669, 200003, 500000,
      1000039, 2000083, 5000101, 10000103, 20000033};
  int N = prime_numbers.size();
  for (int i = 0; i < N; ++i) {
    if (value <= prime_numbers[i]) {
      return prime_numbers[i];
    }
  }
  return prime_numbers[N - 1];
}

std::shared_ptr<UBODT> UBODT::read_ubodt_file(const std::string &filename,
		NETWORK::NetworkGraph graph, int multiplier) {
  std::shared_ptr<UBODT> ubodt = nullptr;
  auto start_time = UTIL::get_current_time();
  if(!UTIL::file_exists(filename)) {
	  int buckets = find_prime_number(graph.get_num_vertices()*graph.get_num_vertices());
	  ubodt = std::make_shared<UBODT>(buckets, multiplier, graph);
  } else if (UTIL::check_file_extension(filename,"bin")){
    ubodt = read_ubodt_binary(filename,graph,multiplier);
  } else if (UTIL::check_file_extension(filename,"csv,txt")) {
    ubodt = read_ubodt_csv(filename, graph, multiplier);
  } else {
    std::string message = (boost::format("File format not supported: %1%") % filename).str();
    SPDLOG_CRITICAL(message);
    throw std::runtime_error(message);
  }
  auto end_time = UTIL::get_current_time();
  double duration = UTIL::get_duration(start_time,end_time);
  SPDLOG_INFO("Read UBODT file in {} seconds",duration);
  return ubodt;
}

std::shared_ptr<UBODT> UBODT::read_ubodt_csv(const std::string &filename,
                                             const NETWORK::NetworkGraph graph,
                                             int multiplier) {
  SPDLOG_INFO("Reading UBODT file (CSV format) from {}", filename);
  long rows = estimate_ubodt_rows(filename);
  int buckets = find_prime_number(rows / LOAD_FACTOR);
  SPDLOG_TRACE("Estimated buckets {}", buckets);
  int progress_step = 1000000;
  std::shared_ptr<UBODT> table = std::make_shared<UBODT>(buckets, multiplier, graph);
  FILE *stream = fopen(filename.c_str(), "r");
  long NUM_ROWS = 0;
  char line[BUFFER_LINE];
  if (fgets(line, BUFFER_LINE, stream)) {
    SPDLOG_TRACE("Header line skipped.");
  }
  while (fgets(line, BUFFER_LINE, stream)) {
    ++NUM_ROWS;
    Record *r = (Record *) malloc(sizeof(Record));
    /* Parse line into a Record */
    sscanf(
        line, "%d;%d;%d;%d;%d;%lf",
        &r->source,
        &r->target,
        &r->first_n,
        &r->prev_n,
        &r->next_e,
        &r->cost
    );
    r->next = nullptr;
    table->insert(r);
    if (NUM_ROWS % progress_step == 0) {
      SPDLOG_INFO("Read rows {}", NUM_ROWS);
    }
  }
  fclose(stream);
  double lf = NUM_ROWS / (double) buckets;
  SPDLOG_TRACE("Estimated load factor #elements/#tablebuckets {}", lf);
  if (lf > 10) { SPDLOG_WARN("Load factor is too large."); }
  SPDLOG_INFO("Finish reading UBODT with rows {}", NUM_ROWS);
  return table;
}

std::shared_ptr<UBODT> UBODT::read_ubodt_binary(const std::string &filename,
                                                 const NETWORK::NetworkGraph graph,
                                                 int multiplier) {
  SPDLOG_INFO("Reading UBODT file (binary format) from {}", filename);
  long rows = estimate_ubodt_rows(filename);
  int progress_step = 1000000;
  SPDLOG_TRACE("Estimated rows is {}", rows);
  int buckets = find_prime_number(rows / LOAD_FACTOR);
  std::shared_ptr<UBODT> table = std::make_shared<UBODT>(buckets, multiplier, graph);
  long NUM_ROWS = 0;
  std::ifstream ifs(filename.c_str());
  // Check byte offset
  std::streampos archiveOffset = ifs.tellg();
  std::streampos streamEnd = ifs.seekg(0, std::ios_base::end).tellg();
  ifs.seekg(archiveOffset);
  boost::archive::binary_iarchive ia(ifs);
  while (ifs.tellg() < streamEnd) {
    ++NUM_ROWS;
    Record *r = (Record *) malloc(sizeof(Record));
    ia >> r->source;
    ia >> r->target;
    ia >> r->first_n;
    ia >> r->prev_n;
    ia >> r->next_e;
    ia >> r->cost;
    r->next = nullptr;
    table->insert(r);
    if (NUM_ROWS % progress_step == 0) {
      SPDLOG_INFO("Read rows {}", NUM_ROWS);
    }
  }
  ifs.close();
  double lf = NUM_ROWS / (double) buckets;
  SPDLOG_TRACE("Estimated load factor #elements/#tablebuckets {}", lf);
  if (lf > 10) {
    SPDLOG_WARN("Load factor is too large.");
  }
  SPDLOG_INFO("Finish reading UBODT with rows {}", NUM_ROWS);
  return table;
}
