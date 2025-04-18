a = input()
b = input()

if (suma := sum(list(map(int, a)))) == (sum_b := sum(list(map(int, b)))):
    print("YES")
    
print(suma, sum_b)
print("NO")

# 0000000000010000000000000011100100101010000100100000100010011000110000011101001001000001100000000000