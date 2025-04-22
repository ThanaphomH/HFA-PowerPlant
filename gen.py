n = 25000

# ring gen
# with open('input.txt', 'w') as f:
#     f.write(f"{n}\n")
#     f.write(f"{n}\n")
#     for i in range(n-1):
#         f.write(f"{i} {i+1}\n")
#     f.write(f"{n-1} {0}\n")
    
# rand gen
import random

n = 10000
m = 25000

with open('input.txt', 'w') as f:
    f.write(f"{n}\n")
    f.write(f"{m}\n")
    adj = [[] for _ in range(n)]
    for i in range(m):
        a = random.randint(0, n-1)
        b = random.randint(0, n-1)
        while a == b or b in adj[a] or a in adj[b]:
            b = random.randint(0, n-1)
        f.write(f"{a} {b}\n")
        adj[a].append(b)
    
    for i in range(n):
        if len(adj[i]) == 0:
            b = random.randint(0, n-1)
            while b == i or b in adj[i] or i in adj[b] or len(adj[b]) == 0:
                b = random.randint(0, n-1)
            f.write(f"{i} {b}\n")
            m += 1
            adj[i].append(b)
            

    # rewrite m in row 2
    f.seek(0, 0)
    f.write(f"{n}\n")
    f.write(f"{m}\n")