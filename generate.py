from random import randint

print(
    "enter 0 to generate testcases with n=10, enter other to generate those with n=25:"
)
size = input()

if size == "0":
    for count in range(0, 10):
        with open(
            f'./tests/test-{count}-{"MST" if count<4 else "OPTTSP" if count<8 else "FASTTSP"}.txt',
            "w",
        ) as file:
            n = 10
            file.write(f"{n}\n")
            file.write(f"0 {randint(-10,0)}\n")
            file.write(f"{randint(-10,0)} 0\n")
            for i in range(2, n):
                file.write(f"{randint(-10,10)} {randint(-10,10)}\n")
else:
    for count in range(0, 1):
        with open(
            f"./tests/large_test-OPTTSP.txt",
            "w",
        ) as file:
            n = 21
            file.write(f"{n}\n")
            file.write(f"0 {randint(-100,0)}\n")
            file.write(f"{randint(-100,0)} 0\n")
            for i in range(2, n):
                file.write(f"{randint(-100,100)} {randint(-100,100)}\n")
