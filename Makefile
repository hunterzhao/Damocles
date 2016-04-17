damo : main.o
	cc -g -o damo main.o

main.o : main.c sock.c
	cc -g -c main.c

clean :
	rm main.o damo
