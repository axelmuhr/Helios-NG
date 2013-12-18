CC=	c
CFLAGS=	-D_BSD
OBJECTS= sok.o readscrn.o showscrn.o play.o save.o score.o

sok: $(OBJECTS)
	$(CC) $(CFLAGS) -o sok $(OBJECTS) -lcurses -ltermcap

sok.o: sok.c sokoban.h
readscrn.o: readscrn.c sokoban.h
showscrn.o: showscrn.c sokoban.h bitmaps.h
play.o: play.c sokoban.h
save.o: save.c sokoban.h
score.o: score.c sokoban.h

install:
	-mkdir	/helios/games
	-mkdir	/helios/games/lib
	-mkdir	/helios/games/lib/sokoban
	-mkdir	/helios/games/lib/sokoban/screens
	-mkdir	/helios/games/lib/sokoban/savegames
	cp sok	/helios/games
	cp	./screens/* /helios/games/lib/sokoban/screens
	cp	./sok.score /helios/games/lib/sokoban
