prog: enemy.o main.o
	gcc enemy.o main.o -o prog -g -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer -lm

main.o: main.c
	gcc -c main.c -g 

enemy.o: enemy.c
	gcc -c enemy.c -g
