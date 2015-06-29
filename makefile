All:main
main:main.o navigation.o filesystem.o function.o syntaxtree.o
	gcc -o main main.o navigation.o filesystem.o function.o syntaxtree.o -lreadline -lhistory -lncursesw -lmenuw -lpanelw
main.o:main.c
	gcc -c main.c
navigation.o:navigation.c navigation.h
	gcc -c navigation.c
filesystem.o:filesystem.c filesystem.h
	gcc -c filesystem.c
function.o:function.c function.h
	gcc -c function.c
syntaxtree.o:syntaxtree.c syntaxtree.h
	gcc -c syntaxtree.c

clean:
	rm main main.o filesystem.o function.o syntaxtree.o navigation.o
