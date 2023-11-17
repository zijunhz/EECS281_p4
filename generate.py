from random import randint

n = 10
print(n)
print(f"0 {randint(-10,0)}")
print(f"{randint(-10,0)} 0")
for i in range(2, n):
    print(f"{randint(-10,10)} {randint(-10,10)}")
