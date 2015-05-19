All:main
main:main.o filesystem.o function.o syntaxtree.o
	cc -o main main.o filesystem.o function.o syntaxtree.o
main.o:main.c
	cc -c main.c
filesystem.o:filesystem.c
	cc -c filesystem.c
function.o:function.c
	cc -c function.c
syntaxtree.o:syntaxtree.c
	cc -c syntaxtree.c

clean:
	rm main main.o filesystem.o function.o syntaxtree.o
