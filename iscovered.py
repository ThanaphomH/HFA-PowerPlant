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

def verify_coverage(n, adj, output):
    # Parse output string to get cities with plants
    if len(output) != n:
        return False, "Output length does not match number of nodes"
    plants = [i for i in range(n) if output[i] == '1']
    
    # Mark covered cities
    covered = [False] * n
    for city in plants:
        covered[city] = True  # City with plant is covered
        for neighbor in adj[city]:
            covered[neighbor] = True  # Neighbors are covered
    
    # Check if all cities are covered
    if not all(covered):
        uncovered = [i for i in range(n) if not covered[i]]
        return False, f"Uncovered cities: {uncovered}"
    
    return True, "All cities are covered"

def main():
    # Read graph
    n, adj = read_graph()
    # Read output string (in practice, this could be passed differently)
    output = input("Enter output string: ")
    # Count 1's in output
    count_ones = output.count('1')
    print("Number of power plants:", count_ones)
    
    # Verify coverage
    is_valid, message = verify_coverage(n, adj, output)
    print(message)
    print("Valid solution:", is_valid)

if __name__ == "__main__":
    main()