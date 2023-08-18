#include "MaximalCardinalityMatching.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

pair<MaximalCardinalityMatching::Graph, vector<double>> CreateRandomGraph()
{
    // random seed
    int x;
    cin >> x;
    srand(x);

    // Please see Graph.h for a description of the interface
    int n = 50;

    MaximalCardinalityMatching::Graph G(n);
    vector<double> cost;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (rand() % 10 == 0)
            {
                G.AddEdge(i, j);
                cost.push_back(rand() % 1000);
            }

    return make_pair(G, cost);
}

MaximalCardinalityMatching::Graph ReadGraph(string filename)
{
    // Please see Graph.h for a description of the interface

    ifstream file;
    file.open(filename.c_str());

    string s;
    getline(file, s);
    stringstream ss(s);
    int n;
    ss >> n;
    getline(file, s);
    ss.str(s);
    ss.clear();
    int m;
    ss >> m;

    MaximalCardinalityMatching::Graph G(n);
    for (int i = 0; i < m; i++)
    {
        getline(file, s);
        ss.str(s);
        ss.clear();
        int u, v;
        ss >> u >> v;

        G.AddEdge(u, v);
    }

    file.close();
    return G;
}

pair<MaximalCardinalityMatching::Graph, vector<double>> ReadWeightedGraph(string filename)
{
    // Please see Graph.h for a description of the interface

    ifstream file;
    file.open(filename.c_str());

    string s;
    getline(file, s);
    stringstream ss(s);
    int n;
    ss >> n;
    getline(file, s);
    ss.str(s);
    ss.clear();
    int m;
    ss >> m;

    MaximalCardinalityMatching::Graph G(n);
    vector<double> cost(m);
    for (int i = 0; i < m; i++)
    {
        getline(file, s);
        ss.str(s);
        ss.clear();
        int u, v;
        double c;
        ss >> u >> v >> c;

        G.AddEdge(u, v);
        cost[G.GetEdgeIndex(u, v)] = c;
    }

    file.close();
    return make_pair(G, cost);
}

void MinimumCostPerfectMatchingExample(string filename)
{
    MaximalCardinalityMatching::Graph G;
    vector<double> cost;

    // Read the graph
    pair<MaximalCardinalityMatching::Graph, vector<double>> p = ReadWeightedGraph(filename);
    // pair< Graph, vector<double> > p = CreateRandomGraph();
    G = p.first;
    cost = p.second;

    // Create a MaximalCardinalityMatching instance passing the graph
    MaximalCardinalityMatching M(G);

    // Pass the costs to solve the problem
    pair<list<int>, double> solution = M.SolveMinimumCostPerfectMatching(cost);

    list<int> matching = solution.first;
    double obj = solution.second;

    cout << "Optimal matching cost: " << obj << endl;
    cout << "Edges in the matching:" << endl;
    for (list<int>::iterator it = matching.begin(); it != matching.end(); it++)
    {
        pair<int, int> e = G.GetEdge(*it);

        cout << e.first << " " << e.second << endl;
    }
}

void MaximumMatchingExample(string filename)
{
    MaximalCardinalityMatching::Graph G = ReadGraph(filename);
    MaximalCardinalityMatching M(G);

    list<int> matching;
    matching = M.SolveMaximumMatching();

    cout << "Number of edges in the maximum matching: " << matching.size() << endl;
    cout << "Edges in the matching:" << endl;
    for (list<int>::iterator it = matching.begin(); it != matching.end(); it++)
    {
        pair<int, int> e = G.GetEdge(*it);

        cout << e.first << " " << e.second << endl;
    }
}
