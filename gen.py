n = 25000

# open write file
with open('input.txt', 'w') as f:
    f.write(f"{n}\n")
    f.write(f"{n}\n")
    for i in range(n-1):
        f.write(f"{i} {i+1}\n")
    f.write(f"{n-1} {0}\n")