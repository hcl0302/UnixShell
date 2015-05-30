All:main
main:main.o filesystem.o function.o syntaxtree.o
	cc -Wall -o main main.o filesystem.o function.o syntaxtree.o
main.o:main.c
	cc -Wall -c main.c
filesystem.o:filesystem.c
	cc -Wall -c filesystem.c
function.o:function.c
	cc -Wall -c function.c
syntaxtree.o:syntaxtree.c
	cc -Wall -c syntaxtree.c

clean:
	rm main main.o filesystem.o function.o syntaxtree.o
