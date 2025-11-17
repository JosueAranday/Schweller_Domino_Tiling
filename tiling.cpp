#include "tiling.h"
#include "vertex.h"

using namespace std;


// Finds a (shortest according to edge length) augmenting path
// from s to t in a graph with vertex set V.
// Returns whether there is an augmenting path.
bool augmenting_path(Vertex* s, Vertex* t, unordered_set<Vertex*> V, vector<Vertex*>& P)	// Runs in O(V + E) time
{
	// Check that s and t aren't nullptr
	if (s == nullptr || t == nullptr)
	{
		cerr << "augmenting_path() was passed nullptr s or t." << endl;
		abort();
	}

	// Check that s and t are in the graph
	if (V.find(s) == V.end() || V.find(t) == V.end())
	{
		cerr << "augmenting_path() was passed s or t not in V." << endl;
		abort();
	}

	// Check that every vertex has valid neighs/weights.
	for (Vertex* v : V)
		for (Vertex* vn : v->neighs)
			if (v->weights.find(vn) == v->weights.end())
			{
				cerr << "augmenting_path() was passed invalid vertex." << endl;
				abort();
			}

    // Since augmenting paths should have the fewest edges,
	// not the minimum weight, run BFS.
	queue<Vertex*> Q;
	Q.push(s);

	unordered_set<Vertex*> R;
	R.clear(); 
	R.insert(s);

	unordered_map<Vertex*, Vertex*> prev;

	while (!Q.empty())
	{
		Vertex* cur = Q.front();
		Q.pop();

		for (Vertex* nei : cur->neighs)
		{
			// Must have positive edge weight
			if (cur->weights[nei] == 0)
				continue;

			if (R.find(nei) == R.end())
			{
				Q.push(nei);
				R.insert(nei);
				prev[nei] = cur; 
			}
		}
	}      

    // If BFS never reached t
    if (R.find(t) == R.end())
		return false;

    // Reconstruct shortest path backwards
    P.clear();
    P.push_back(t);
	while (P[P.size()-1] != s)
		P.push_back(prev[P[P.size()-1]]);

    // Reverse shortest path
     for (int i = 0; i < P.size()/2; ++i)
		 swap(P[i], P[P.size()-1-i]);
	 
	 return true;
}

// Returns the maximum flow from s to t in a weighted graph with vertex set V.
// Assumes all edge weights are non-negative.
int max_flow(Vertex* s, Vertex* t, unordered_set<Vertex*> V)
{
	// If s or t is invalid.
	if (s == nullptr || t == nullptr)
	{
		cerr << "max_flow() was passed nullptr s or t." << endl;
		abort();
	}

	// If s or t is not in the vertex set.
	if (V.find(s) == V.end() || V.find(t) == V.end())
	{
		cerr << "max_flow() was passed s or t not in V." << endl;
		abort();
	}

	// Check that every vertex has valid neighs/weights.
	for (Vertex* v : V)
		for (Vertex* vn : v->neighs)
			if (v->weights.find(vn) == v->weights.end())
			{
				cerr << "max_flow() was passed invalid vertex." << endl;
				abort();
			}

	// Create a deep copy of V to use as the residual graph
	unordered_set<Vertex*> resV;
	unordered_map<Vertex*, Vertex*> C; // Maps vertices in V to copies in resV
	for (Vertex* vp : V)
	{
		Vertex* rp = new Vertex;
		resV.insert(rp);
		C[vp] = rp;
	}
	for (Vertex* vp : V)
		for (Vertex* np : vp->neighs)
		{
			C[vp]->neighs.insert(C[np]);
			C[vp]->weights[C[np]] = vp->weights[np];
		}

	// Add any missing necessary "back" edges. 
	for (Vertex* vp : V)
		for (Vertex* np : vp->neighs)
		{
			if (C[np]->neighs.find(C[vp]) == C[np]->neighs.end())
			{
				C[np]->neighs.insert(C[vp]);
				C[np]->weights[C[vp]] = 0;
			}
		}

	// Run Edmonds-Karp
	while (true)
	{
		// Find an augmenting path
        vector<Vertex*> P;
        if (!augmenting_path(C[s], C[t], resV, P))
			break;  
        // Update residual graph
        for (int i = 0; i < P.size()-1; ++i)
        {
			--((*(resV.find(P[i])))->weights[P[i+1]]);
            ++((*(resV.find(P[i+1])))->weights[P[i]]);
        }
    }
	
	// Compute actual flow amount
    int flow = 0;
    for (Vertex* snp : C[s]->neighs)
		flow += 1 - C[s]->weights[snp];

    // Delete residual graph
    for (Vertex* vp : resV)
		delete vp;
	
	return flow;
}


bool has_tiling(string floor)
{
	size_t pos = floor.find('\n');	// Find first newline character
	size_t rowLength = (pos == string::npos) ? floor.size() : pos;	// Including the newline character

	size_t floorSize = floor.size();
	unordered_map<int, Vertex*> vertexMap;

	// Create vertices for all empty squares and separate into black and white sets
	vector<Vertex*> blackVertices;
	vector<Vertex*> whiteVertices;


	int xCoor = 0;
	int yCoor = 0;
	int counter = 0;
	int gridIndex = 0;
	int blackCount = 0;
	int whiteCount = 0;
	//int colPlusRow = 0;

	while (counter < floor.size()) {
		char curr = floor[counter];

		if (curr == '\n') {
			yCoor++;
			xCoor = 0;
			counter++;
			continue;
		}

		if (curr == ' ') {
			Vertex* v = new Vertex();
			vertexMap[gridIndex] = v;

			if ((xCoor + yCoor) % 2 == 0) {
				blackVertices.push_back(v);
				blackCount++;
			}
			else {
				whiteVertices.push_back(v);
				whiteCount++;
			}
		}

		xCoor++;
		counter++;
		gridIndex++;
	}

	int emptyCount = blackCount + whiteCount;
	if (emptyCount % 2 != 0) {
		for (auto& p : vertexMap) delete p.second;
		return false;
	}

	// If different numbers of black and white squares, return false
	if (blackCount != whiteCount) {
		for (auto& p : vertexMap) delete p.second;	// Clean up dynamically allocated memory
		return false;	// Since there are different numbers of black and white squares, it cant be fully tiled
	}

	// Create start and target vertices
	Vertex* startV = new Vertex();
	Vertex* targetV = new Vertex();

	// Connect start to all whites
	for (Vertex* w : whiteVertices) {
		startV->neighs.insert(w);
		startV->weights[w] = 1;
	}

	// Connect blacks to target
	for (Vertex* b : blackVertices) {
		b->neighs.insert(targetV);
		b->weights[targetV] = 1;
	}

	// Now connect the vertices
	for (auto const& pair : vertexMap) {
		int index = pair.first;
		Vertex* currV = pair.second;

		int x = index % rowLength;	// col
		int y = index / rowLength;	// row

		if ((x + y) % 2 == 0) continue;	// Only from whites

		auto connect_if_black = [&](int gx, int gy) { // Helper inline function
			if (gx < 0 || gy < 0) return;
			int nIndex = gy * rowLength + gx;

			auto it = vertexMap.find(nIndex);
			if (it == vertexMap.end()) return;

			Vertex* neigh = it->second;

			if ((gx + gy) % 2 == 0) {
				currV->neighs.insert(neigh);
				currV->weights[neigh] = 1;
			}
		};
			
		// right, left, down, up
		connect_if_black(x + 1, y);
		connect_if_black(x - 1, y);
		connect_if_black(x, y + 1);
		connect_if_black(x, y - 1);

	}
	// Create an unordered_set<Vertex*> V containing s, t, and all grid vertices (from vertexMap)
	unordered_set<Vertex*> V;
	V.insert(startV);
	for (auto const& pair : vertexMap) {
		V.insert(pair.second);
	}
	V.insert(targetV);
	// Call the algoirithm to get max flow
	int flow = max_flow(startV, targetV, V);

	// Determine if max flow equals number of white vertices
	if (flow == whiteCount) {
		// Clean up dynamically allocated memory
		for (auto const& pair : vertexMap) {
			delete pair.second;
		}
		delete startV;
		delete targetV;
		return true;
	}

	// Clean up dynamically allocated memory
	for (auto const& pair : vertexMap) {
		delete pair.second;
	}
	delete startV;
	delete targetV;

	return false;
}