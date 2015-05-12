All:main
main:main.o filesystem.o
	cc -o main main.o filesystem.o
main.o:main.c
	cc -c main.c
filesystem.o:filesystem.c
	cc -c filesystem.c

clean:
	rm main main.o filesystem.o
