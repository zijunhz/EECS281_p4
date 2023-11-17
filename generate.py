from random import randint

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
