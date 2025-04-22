n = int(input())
m = int(input())

adj = [[] for _ in range(n)]
for _ in range(m):
    u, v = map(int, input().split())
    adj[u].append(v)
    adj[v].append(u)  # Undirected graph
    
# check if graph is connected
def bfs(start, visited):
    queue = [start]
    visited[start] = True
    while queue:
        node = queue.pop(0)
        for neighbor in adj[node]:
            if not visited[neighbor]:
                visited[neighbor] = True
                queue.append(neighbor)

visited = [False] * n
bfs(0, visited)

if all(visited):
    print("The graph is connected.")
else:
    print("The graph is not connected.")
    for i in range(n):
        if not visited[i]:
            print(f"Unvisited node: {i}")
           