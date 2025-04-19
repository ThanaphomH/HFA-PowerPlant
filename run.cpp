#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <set>
#include <fstream>
#include "Highs.h"

using namespace std;

// Function to read graph input
pair<int, vector<vector<int>>> read_graph(ifstream& in) {
    int n, m;
    if (!(in >> n >> m)) {
        throw runtime_error("Failed to read n or m");
    }

    vector<vector<int>> adj(n);
    for (auto& neighbors : adj) {
        neighbors.reserve(max(4, m / n + 1)); // Heuristic: avg degree or min 4
    }

    for (int i = 0; i < m; ++i) {
        int u, v;
        if (!(in >> u >> v)) {
            throw runtime_error("Failed to read edge " + to_string(i));
        }
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    return {n, adj};
}

// Function to find a greedy dominating set
vector<int> greedy_dominating_set(int n, const vector<vector<int>>& adj) {
    vector<bool> covered(n, false); // Track covered vertices
    vector<int> solution; // Store the solution (selected vertices)
    vector<int> uncovered_count(n, 0); // Track number of uncovered neighbors for each vertex   
    priority_queue<pair<int, int>> pq; // Priority queue to select vertex with max uncovered neighbors {uncovered_count, vertex}
    set<int> active_vertices; // Track vertices that are not yet covered or selected

    // Initialize uncovered counts and priority queue
    for (int i = 0; i < n; ++i) {
        uncovered_count[i] = adj[i].size() + 1; // Include self
        pq.push({uncovered_count[i], i});
        active_vertices.insert(i);
    }

    while (!active_vertices.empty()) {
        // Get vertex with maximum uncovered neighbors
        while (!pq.empty() && covered[pq.top().second]) {
            pq.pop(); 
        }
        if (pq.empty()) break; // All vertices covered

        int best_vertex = pq.top().second;
        pq.pop();

        if (covered[best_vertex]) continue; // Skip if already covered

        // Select the vertex
        solution.push_back(best_vertex);
        covered[best_vertex] = true;
        active_vertices.erase(best_vertex);

        // Update neighbors
        for (int j : adj[best_vertex]) {
            if (!covered[j]) {
                covered[j] = true;
                active_vertices.erase(j);
                // Update uncovered counts for neighbors of j
                for (int k : adj[j]) {
                    if (!covered[k]) {
                        --uncovered_count[k];
                        pq.push({uncovered_count[k], k});
                    }
                }
            }
        }

        // Update uncovered counts for vertices that had best_vertex as a neighbor
        for (int j : adj[best_vertex]) {
            for (int k : adj[j]) {
                if (!covered[k]) {
                    --uncovered_count[k];
                    pq.push({uncovered_count[k], k});
                }
            }
        }
    }

    return solution;
}

// Function to solve power plant placement using HiGHS
string solve_power_plants(int n, const vector<vector<int>>& adj) {

    // Create HiGHS instance
    Highs highs;
    highs.setOptionValue("log_to_console", false); // Suppress solver output
    highs.setOptionValue("threads", 6); // Set number of threads
    highs.setOptionValue("mip_rel_gap", 0.1); // MIP gap
    highs.setOptionValue("mip_heuristic_effort", 0.5); // Set heuristic effort
    highs.setOptionValue("mip_improving_solution", false);
    highs.setOptionValue("presolve", "on");

    vector<int> greedy_solution = greedy_dominating_set(n, adj);
    HighsSolution start_solution;
    start_solution.col_value.resize(n, 0.0);
    for (int i : greedy_solution) start_solution.col_value[i] = 1.0;

    // Pass starting solution (HiGHS may require MIP start API in future versions)
    highs.setSolution(start_solution);

    // Initialize model
    HighsLp lp;
    lp.num_col_ = n;
    lp.num_row_ = n;
    lp.sense_ = ObjSense::kMinimize;

    // Objective: Minimize sum of x[i]
    vector<double> col_cost(n, 1.0); // Coefficient of 1 for each x[i]
    lp.col_cost_ = col_cost;

    // Variables: x[i] are binary (0 or 1)
    vector<HighsVarType> integrality(n, HighsVarType::kInteger);
    lp.integrality_ = integrality; // Set integrality directly
    vector<double> col_lower(n, 0.0);
    vector<double> col_upper(n, 1.0);
    lp.col_lower_ = col_lower;
    lp.col_upper_ = col_upper;

    // Constraints: Each city i must be covered (x[i] + sum(x[j] for j in N(i)) >= 1)
    vector<int> start(n + 1, 0); // CSR format for constraint matrix
    vector<int> index; // Column indices
    vector<double> value; // Coefficients
    vector<double> row_lower(n, 1.0); // Lower bound >= 1
    vector<double> row_upper(n, kHighsInf); // No upper bound

    size_t total_nnz = n + std::accumulate(adj.begin(), adj.end(), size_t(0),
                                      [](size_t sum, const auto& v) { return sum + v.size(); });
    index.reserve(total_nnz);
    value.reserve(total_nnz);

    vector<vector<int>> local_index(n), local_value(n);
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        local_index[i].push_back(i);
        local_value[i].push_back(1.0);
        for (int j : adj[i]) {
            local_index[i].push_back(j);
            local_value[i].push_back(1.0);
        }
    }
    for (int i = 0; i < n; ++i) {
        index.insert(index.end(), local_index[i].begin(), local_index[i].end());
        value.insert(value.end(), local_value[i].begin(), local_value[i].end());
        start[i + 1] = index.size();
    }

    // // Add clique inequalities
    // for (int i = 0; i < n; ++i) {
    //     for (int j : adj[i]) {
    //         for (int k : adj[j]) {
    //             if (k > j && find(adj[i].begin(), adj[i].end(), k) != adj[i].end()) {
    //                 index.push_back(i); index.push_back(j); index.push_back(k);
    //                 value.push_back(1.0); value.push_back(1.0); value.push_back(1.0);
    //                 start.push_back(index.size());
    //                 row_lower.push_back(1.0);
    //                 row_upper.push_back(kHighsInf);
    //                 lp.num_row_++;
    //             }
    //         }
    //     }
    // }

    // // Symmetry Breaking
    // for (int i = 0; i < n; ++i) {
    //     for (int j = i + 1; j < n; ++j) {
    //         if (adj[i] == adj[j]) { // Identical neighborhoods
    //             index.push_back(i); index.push_back(j);
    //             value.push_back(1.0); value.push_back(-1.0);
    //             start.push_back(index.size());
    //             row_lower.push_back(0.0); // x_i <= x_j
    //             row_upper.push_back(0.0);
    //             lp.num_row_++;
    //         }
    //     }
    // }

    // // Strengthen Constraints
    // for (int i = 0; i < n; ++i) {
    //     if (adj[i].size() == 1) {
    //         int j = adj[i][0];
    //         index.push_back(i); index.push_back(j);
    //         value.push_back(1.0); value.push_back(1.0);
    //         start.push_back(index.size());
    //         row_lower.push_back(1.0); // x_i + x_j >= 1
    //         row_upper.push_back(kHighsInf);
    //         lp.num_row_++;
    //     }
    // }

    lp.a_matrix_.format_ = MatrixFormat::kColwise;
    lp.a_matrix_.start_ = start;
    lp.a_matrix_.index_ = index;
    lp.a_matrix_.value_ = value;
    lp.row_lower_ = row_lower;
    lp.row_upper_ = row_upper;

    // Pass model to solver
    highs.passModel(lp);

    // Solve the problem
    highs.run();

    // Check solution status
    HighsModelStatus model_status = highs.getModelStatus();
    if (model_status == HighsModelStatus::kOptimal) {
        // Get solution
        vector<double> solution = highs.getSolution().col_value;
        string result(n, '0');
        for (int i = 0; i < n; ++i) {
            if (solution[i] > 0.5) { // Binary variable
                result[i] = '1';
            }
        }

        return result;
    } else {
        // No optimal solution found, return all ones
        
        string result(n, '1');
        return result;
    }
}

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        return 1;
    }
    ifstream in(argv[1]);  
    ofstream out(argv[2]); 

    // Read input
    auto [n, adj] = read_graph(in);

    // Solve ILP and get binary string
    string result = solve_power_plants(n, adj);
    if (!result.empty()) {
        out << result << '\n';
    }

    return 0;
}