all:
	g++ -pthread server.c -o server
	g++ -pthread random_variable.c client.c -o client -lm
