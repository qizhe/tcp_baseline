all:
	gcc -pthread server.c -o server
	gcc -pthread random_variable.c client.c -o client -lm
