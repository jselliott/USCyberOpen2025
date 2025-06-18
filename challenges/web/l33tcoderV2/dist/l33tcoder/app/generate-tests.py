import random

F = open("files/test_cases.txt","w")

for i in range(100):
    X = [random.randrange(0,100) for i in range(random.randrange(1,20))]
    Y = sorted(X)

    F.write("(%s,%s)\n" % (str(X),str(Y)))

F.close()

print("** Generated Random Test Cases **")