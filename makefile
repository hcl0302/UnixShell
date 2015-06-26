All:main
main:main.o navigation.o filesystem.o function.o syntaxtree.o
	gcc -Wall -g -o main main.o navigation.o filesystem.o function.o syntaxtree.o -lreadline -lhistory -lncursesw -lmenuw -lpanelw
main.o:main.c
	gcc -Wall -g -c main.c
navigation.o:navigation.c navigation.h
	gcc -Wall -g -c navigation.c
filesystem.o:filesystem.c filesystem.h
	gcc -Wall -g -c filesystem.c
function.o:function.c function.h
	gcc -Wall -g -c function.c
syntaxtree.o:syntaxtree.c syntaxtree.h
	gcc -Wall -g -c syntaxtree.c

clean:
	rm main main.o filesystem.o function.o syntaxtree.o navigation.o
