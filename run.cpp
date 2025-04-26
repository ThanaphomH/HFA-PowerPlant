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
pair<int, vector<vector<int>>> read_graph(const char* filename) {
    // Open file
    FILE* file = fopen(filename, "r");
    if (!file) {
        throw runtime_error("Failed to open file: " + string(filename));
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read entire file into buffer
    vector<char> buffer(file_size + 1);
    size_t bytes_read = fread(buffer.data(), 1, file_size, file);
    buffer[bytes_read] = '\0'; // Null-terminate for parsing
    fclose(file);

    if (bytes_read != file_size) {
        throw runtime_error("Failed to read file");
    }

    // Parse buffer
    int n, m;
    const char* ptr = buffer.data();
    
    // Read n and m
    auto read_int = [&ptr]() {
        while (*ptr && (*ptr < '0' || *ptr > '9')) ++ptr; // Skip non-digits
        int val = 0;
        while (*ptr >= '0' && *ptr <= '9') {
            val = val * 10 + (*ptr - '0');
            ++ptr;
        }
        return val;
    };

    n = read_int();
    m = read_int();

    if (n <= 0 || m < 0) {
        throw runtime_error("Invalid n or m");
    }

    // Initialize adjacency list
    vector<vector<int>> adj(n);
    for (auto& neighbors : adj) {
        neighbors.reserve(max(4, m / n + 1)); // Same heuristic
    }

    // Read m edges
    for (int i = 0; i < m; ++i) {
        int u = read_int();
        int v = read_int();
        if (u < 0 || u >= n || v < 0 || v >= n) {
            throw runtime_error("Invalid vertex in edge " + to_string(i));
        }
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    return {n, adj};
}

// Function to find a greedy dominating set 
vector<double> greedy_dominating_set(int n, const vector<vector<int>>& adj) {
    vector<bool> covered(n, false);
    vector<double> solution(n, 0.0); // Initialize with 0.0 for HiGHS
    vector<int> uncovered_count(n, 0);
    priority_queue<pair<int, int>> pq;
    set<int> active_vertices;

    // Initialize priority queue and counts
    for (int i = 0; i < n; ++i) {
        uncovered_count[i] = adj[i].size() + 1; // Self + neighbors
        pq.push({uncovered_count[i], i});
        active_vertices.insert(i);
    }

    // Greedy selection
    while (!active_vertices.empty()) {
        while (!pq.empty() && covered[pq.top().second]) {
            pq.pop();
        }
        if (pq.empty()) break;

        int best_vertex = pq.top().second;
        pq.pop();

        if (covered[best_vertex]) continue;

        solution[best_vertex] = 1.0; // Mark vertex in solution
        covered[best_vertex] = true;
        active_vertices.erase(best_vertex);

        // Update neighbors
        for (int j : adj[best_vertex]) {
            if (!covered[j]) {
                covered[j] = true;
                active_vertices.erase(j);
                for (int k : adj[j]) {
                    if (!covered[k]) {
                        --uncovered_count[k];
                        pq.push({uncovered_count[k], k});
                    }
                }
            }
        }

        // Update neighbors of neighbors
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
    highs.setOptionValue("threads", 12); // Set number of threads
    highs.setOptionValue("mip_rel_gap", 0.1);
    highs.setOptionValue("mip_heuristic_effort", 0.5);
    highs.setOptionValue("presolve", "on");

    vector<double> start_solution = greedy_dominating_set(n, adj);
    HighsSolution highs_solution;
    highs_solution.col_value = start_solution;
    highs_solution.row_dual.clear();

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

    lp.a_matrix_.format_ = MatrixFormat::kColwise;
    lp.a_matrix_.start_ = start;
    lp.a_matrix_.index_ = index;
    lp.a_matrix_.value_ = value;
    lp.row_lower_ = row_lower;
    lp.row_upper_ = row_upper;

    // Pass model to solver
    highs.passModel(lp);

    // Pass starting solution (HiGHS may require MIP start API in future versions)
    highs.setSolution(highs_solution);

    // Solve the problem
    highs.run();

    // Check solution status
    HighsModelStatus model_status = highs.getModelStatus();
    if (model_status == HighsModelStatus::kOptimal || model_status == HighsModelStatus::kTimeLimit) {
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
    cin.tie(nullptr);
    ios_base::sync_with_stdio(false);
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        return 1;
    }

    // Read input
    auto [n, adj] = read_graph(argv[1]);

    // Solve ILP and get binary string
    string result = solve_power_plants(n, adj);
    
    // Write output using fwrite
    FILE* file = fopen(argv[2], "w");
    if (!result.empty()) {
        fwrite(result.c_str(), 1, result.size(), file);
        fwrite("\n", 1, 1, file);
    }
    fclose(file);

    return 0;
}