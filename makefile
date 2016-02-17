teamWebserver: teamWebserver.o mySocket.o myException.o
	gcc -o teamWebserver teamWebserver.o mySocket.o myException.o

teamWebserver.o: teamWebserver.c
	gcc -c teamWebserver.c

mySocket.o: mySocket.c
	gcc -c mySocket.c

myException.o: myException.c
	gcc -c myException.c

clean:
	rm -f teamWebserver.o mySocket.o myException.o
