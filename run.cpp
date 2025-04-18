#include <iostream>
#include <vector>
#include <string>
#include "Highs.h"

using namespace std;

// Function to read graph input
pair<int, vector<vector<int>>> read_graph() {
    int n, m;
    cin >> n >> m;
    vector<vector<int>> adj(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u); // Undirected graph
    }
    return {n, adj};
}

// Function to solve power plant placement using HiGHS
string solve_power_plants(int n, const vector<vector<int>>& adj) {
    // Create HiGHS instance
    Highs highs;
    highs.setOptionValue("log_to_console", false); // Suppress solver output
    highs.setOptionValue("threads", 4); // Use 4 threads
    highs.setOptionValue("mip_rel_gap", 0.01); // 1% MIP gap

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

    for (int i = 0; i < n; ++i) {
        // Add x[i] to constraint
        index.push_back(i);
        value.push_back(1.0);
        // Add neighbors x[j] for j in adj[i]
        for (int j : adj[i]) {
            index.push_back(j);
            value.push_back(1.0);
        }
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
        cout << "No optimal solution found. Model status: " << highs.modelStatusToString(model_status) << endl;
        return "";
    }
}

int main() {
    // Read input
    auto [n, adj] = read_graph();

    // Solve ILP and get binary string
    string result = solve_power_plants(n, adj);
    if (!result.empty()) {
        cout << result << endl;
    }

    return 0;
}