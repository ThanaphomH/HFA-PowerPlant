from pulp import LpProblem, LpMinimize, LpVariable, lpSum, LpStatus

def read_graph():
    # Read number of nodes and edges
    n = int(input())
    m = int(input())
    # Initialize adjacency list
    adj = [[] for _ in range(n)]
    # Read edges
    for _ in range(m):
        u, v = map(int, input().split())
        adj[u].append(v)
        adj[v].append(u)  # Undirected graph
    return n, adj

def solve_power_plants(n, adj):
    # Create the ILP problem
    model = LpProblem("Power_Plant_Placement", LpMinimize)
    
    # Create binary variables x[i] for each city
    x = {i: LpVariable(f'x_{i}', cat='Binary') for i in range(n)}
    
    # Objective: Minimize sum of x[i]
    model += lpSum(x[i] for i in range(n))
    
    # Constraints: Each city i must be covered (x[i] + sum(x[j] for j in N(i)) >= 1)
    for i in range(n):
        model += x[i] + lpSum(x[j] for j in adj[i]) >= 1, f'cover_{i}'
    
    # Solve the ILP (uses CBC by default)
    model.solve()
    
    # Check if solution is optimal
    if LpStatus[model.status] == 'Optimal':
        # Construct binary string
        result = ['0'] * n
        for i in range(n):
            if x[i].varValue > 0.5:  # Binary variable (0 or 1)
                result[i] = '1'
        return ''.join(result)
    else:
        print("No optimal solution found.")
        return None

def main():
    # Read input
    n, adj = read_graph()
    # Solve ILP and get binary string
    result = solve_power_plants(n, adj)
    if result:
        print(result)

if __name__ == "__main__":
    main()