# CXX=docker run -u $(shell id -u):$(shell id -g) --rm -it -v $(PWD):/workdir -w /workdir builder gcc
CXX=gcc
CFLAGS=-fno-pie -no-pie -fno-stack-protector -O0

.PHONY: all
all: decryptonite_client decryptonite_server

decryptonite_client: storage.o client.c
	$(CXX) -o decryptonite_client client.c storage.o -lcrypto $(CFLAGS)

decryptonite_server: storage.o server.c
	$(CXX) -o decryptonite_server server.c storage.o -lcrypto $(CFLAGS)

storage.o:
	$(CXX) -c storage.c -lcrypto $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o decryptonite_server decryptonite_client
