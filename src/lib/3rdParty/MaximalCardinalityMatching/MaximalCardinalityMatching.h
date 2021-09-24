#pragma once

#include <list>
#include <vector>

#define EVEN 2
#define ODD 1
#define UNLABELED 0
#define EPSILON 0.000001
#define INFINITO 1000000000.0
#define GREATER(A, B) ((A) - (B) > EPSILON)
#define LESS(A, B) ((B) - (A) > EPSILON)
#define EQUAL(A, B) (fabs((A) - (B)) < EPSILON)
#define GREATER_EQUAL(A, B) (GREATER((A), (B)) || EQUAL((A), (B)))
#define LESS_EQUAL(A, B) (LESS((A), (B)) || EQUAL((A), (B)))
#define MIN(A, B) (LESS((A), (B)) ? (A) : (B))
#define MAX(A, B) (LESS((A), (B)) ? (B) : (A))

/*
Modified by Tingyuan LIANG for bettern modularization, better performance and name conflict elimination
*/

class MaximalCardinalityMatching
{
  public:
    class Graph
    {
      public:
        // n is the number of vertices
        // edges is a list of pairs representing the edges (default = empty list)
        Graph(int n, const std::list<std::pair<int, int>> &edges = std::list<std::pair<int, int>>())
            : n(n), m(edges.size()), adjMat(n, std::vector<bool>(n, false)), adjList(n), edges(),
              edgeIndex(n, std::vector<int>(n, -1))
        {
            for (std::list<std::pair<int, int>>::const_iterator it = edges.begin(); it != edges.end(); it++)
            {
                int u = (*it).first;
                int v = (*it).second;

                AddEdge(u, v);
            }
        }

        // Default constructor creates an empty graph
        Graph() : n(0), m(0){};

        // Returns the number of vertices
        inline int GetNumVertices() const
        {
            return n;
        };
        // Returns the number of edges
        inline int GetNumEdges() const
        {
            return m;
        };

        // Given the edge's index, returns its endpoints as a pair
        inline std::pair<int, int> GetEdge(int e) const
        {
            if (e > (int)edges.size())
                throw "Error: edge does not exist";

            return edges[e];
        }

        // Given the endpoints, returns the index
        inline int GetEdgeIndex(int u, int v) const
        {
            if (u > n or v > n)
                throw "Error: vertex does not exist";

            if (edgeIndex[u][v] == -1)
                throw "Error: edge does not exist";

            return edgeIndex[u][v];
        }

        // Adds a new vertex to the graph
        inline void AddVertex()
        {
            for (int i = 0; i < n; i++)
            {
                adjMat[i].push_back(false);
                edgeIndex[i].push_back(-1);
            }
            n++;
            adjMat.push_back(std::vector<bool>(n, false));
            edgeIndex.push_back(std::vector<int>(n, -1));
            adjList.push_back(std::list<int>());
        }

        // Adds a new edge to the graph
        inline void AddEdge(int u, int v)
        {
            if (u > n or v > n)
                throw "Error: vertex does not exist";

            if (adjMat[u][v])
                return;

            adjMat[u][v] = adjMat[v][u] = true;
            adjList[u].push_back(v);
            adjList[v].push_back(u);

            edges.push_back(std::pair<int, int>(u, v));
            edgeIndex[u][v] = edgeIndex[v][u] = m++;
        }

        // Returns the adjacency list of a vertex
        inline const std::list<int> &AdjList(int v) const
        {
            if (v > n)
                throw "Error: vertex does not exist";

            return adjList[v];
        }

        // Returns the graph's adjacency matrix
        inline const std::vector<std::vector<bool>> &AdjMat() const
        {
            return adjMat;
        }

      private:
        // Number of vertices
        int n;
        // Number of edges
        int m;

        // Adjacency matrix
        std::vector<std::vector<bool>> adjMat;

        // Adjacency lists
        std::vector<std::list<int>> adjList;

        // Array of edges
        std::vector<std::pair<int, int>> edges;

        // Indices of the edges
        std::vector<std::vector<int>> edgeIndex;
    };

    /*
    This is a binary heap for pairs of the type (double key, int satellite)
    It is assumed that satellites are unique integers
    This is the case with graph algorithms, in which satellites are vertex or edge indices
     */
    class BinaryHeap
    {
      public:
        BinaryHeap() : satellite(1), size(0){};

        // Resets the structure
        inline void Clear()
        {
            key.clear();
            pos.clear();
            satellite.clear();
        }

        // Inserts (key k, satellite s) in the heap
        inline void Insert(double k, int s)
        {
            // Ajust the structures to fit new data
            if (s >= (int)pos.size())
            {
                pos.resize(s + 1, -1);
                key.resize(s + 1);
                // Recall that position 0 of satellite is unused
                satellite.resize(s + 2);
            }
            // If satellite is already in the heap
            else if (pos[s] != -1)
            {
                throw "Error: satellite already in heap";
            }

            int i;
            for (i = ++size; i / 2 > 0 && GREATER(key[satellite[i / 2]], k); i /= 2)
            {
                satellite[i] = satellite[i / 2];
                pos[satellite[i]] = i;
            }
            satellite[i] = s;
            pos[s] = i;
            key[s] = k;
        }

        // Returns the number of elements in the heap
        inline int Size()
        {
            return size;
        }

        // Deletes the element with minimum key and returns its satellite information
        inline int DeleteMin()
        {
            if (size == 0)
                throw "Error: empty heap";

            int min = satellite[1];
            int slast = satellite[size--];

            int child;
            int i;
            for (i = 1, child = 2; child <= size; i = child, child *= 2)
            {
                if (child < size && GREATER(key[satellite[child]], key[satellite[child + 1]]))
                    child++;

                if (GREATER(key[slast], key[satellite[child]]))
                {
                    satellite[i] = satellite[child];
                    pos[satellite[child]] = i;
                }
                else
                    break;
            }
            satellite[i] = slast;
            pos[slast] = i;

            pos[min] = -1;

            return min;
        }

        // Changes the key of the element with satellite s
        inline void ChangeKey(double k, int s)
        {
            Remove(s);
            Insert(k, s);
        }

        // Removes the element with satellite s
        inline void Remove(int s)
        {
            int i;
            for (i = pos[s]; i / 2 > 0; i /= 2)
            {
                satellite[i] = satellite[i / 2];
                pos[satellite[i]] = i;
            }
            satellite[1] = s;
            pos[s] = 1;

            DeleteMin();
        }

      private:
        std::vector<double> key;    // Given the satellite, this is its key
        std::vector<int> pos;       // Given the satellite, this is its position in the heap
        std::vector<int> satellite; // This is the heap!

        // Number of elements in the heap
        int size;
    };

    // Parametric constructor receives a graph instance
    MaximalCardinalityMatching(const Graph &G);

    // Solves the minimum cost perfect matching problem
    // Receives the a vector whose position i has the cost of the edge with index i
    // If the graph doest not have a perfect matching, a const char * exception will be raised
    // Returns a pair
    // the first element of the pair is a list of the indices of the edges in the matching
    // the second is the cost of the matching
    std::pair<std::list<int>, double> SolveMinimumCostPerfectMatching(const std::vector<double> &cost);

    // Solves the maximum cardinality matching problem
    // Returns a list with the indices of the edges in the matching
    std::list<int> SolveMaximumMatching();

  private:
    // Grows an alternating forest
    void Grow();
    // Expands a blossom u
    // If expandBlocked is true, the blossom will be expanded even if it is blocked
    void Expand(int u, bool expandBlocked);
    // Augments the matching using the path from u to v in the alternating forest
    void Augment(int u, int v);
    // Resets the alternating forest
    void Reset();
    // Creates a blossom where the tip is the first common vertex in the paths from u and v in the hungarian forest
    int Blossom(int u, int v);
    void UpdateDualCosts();
    // Resets all data structures
    void Clear();
    void DestroyBlossom(int t);
    // Uses an heuristic algorithm to find the maximum matching of the graph
    void Heuristic();
    // Modifies the costs of the graph so the all edges have positive costs
    void PositiveCosts();
    std::list<int> RetrieveMatching();

    int GetFreeBlossomIndex();
    void AddFreeBlossomIndex(int i);
    void ClearBlossomIndices();

    // An edge might be blocked due to the dual costs
    bool IsEdgeBlocked(int u, int v);
    bool IsEdgeBlocked(int e);
    // Returns true if u and v are adjacent in G and not blocked
    bool IsAdjacent(int u, int v);

    const Graph &G;

    std::list<int> free; // List of free blossom indices

    std::vector<int> outer; // outer[v] gives the index of the outermost blossom that contains v, outer[v] = v if v is
                            // not contained in any blossom
    std::vector<std::list<int>> deep; // deep[v] is a list of all the original vertices contained inside v, deep[v] = v
                                      // if v is an original vertex
    std::vector<std::list<int>> shallow; // shallow[v] is a list of the vertices immediately contained inside v,
                                         // shallow[v] is empty is the default
    std::vector<int> tip;                // tip[v] is the tip of blossom v
    std::vector<bool> active;            // true if a blossom is being used

    std::vector<int> type;   // Even, odd, neither (2, 1, 0)
    std::vector<int> forest; // forest[v] gives the father of v in the alternating forest
    std::vector<int> root;   // root[v] gives the root of v in the alternating forest

    std::vector<bool> blocked; // A blossom can be blocked due to dual costs, this means that it behaves as if it were
                               // an original vertex and cannot be expanded
    std::vector<double>
        dual; // dual multipliers associated to the blossoms, if dual[v] > 0, the blossom is blocked and full
    std::vector<double> slack; // slack associated to each edge, if slack[e] > 0, the edge cannot be used
    std::vector<int> mate;     // mate[v] gives the mate of v

    int m, n;

    bool perfect;

    std::list<int> forestList;
    std::vector<int> visited;
};
