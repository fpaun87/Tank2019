CFLAGS :=  -Wall -Wextra -fno-omit-frame-pointer -c
debug := 1

ifeq ($(debug),1)
	CFLAGS += -g3 -pg -O0
else
	CFLAGS += -march=native -O3
endif

CFLAGS += -I /usr/local/include
LDFLAGS += -L /usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

EXE_DBG := Tank2019_dbg
EXE_RELEASE := Tank2019
EDITOR_EXE := te

ifeq ($(debug),1)
EXE := $(EXE_DBG)
else
EXE := $(EXE_RELEASE)
endif

SRC := src/main.c src/resource_mgr.c src/play_state.c src/util.c \
src/menu_state.c src/level_state.c src/pause_state.c src/game_over_state.c \
src/timer.c src/bonus.c
OBJS:=$(SRC:.c=.o)

EDITOR_SRC := src/editor/main.c
EDITOR_OBJS := $(EDITOR_SRC:.c=.o)


all: EXE EDITOR_EXE tags
editor: EDITOR_EXE

EXE: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXE)

main.o: main.c
	$(CC) $(CFLAGS) main.c

resource_mgr.o: resource_mgr.c
	$(CC) $(CFLAGS) resource_mgr.c

play_state.o: play_state.c
	$(CC) $(CFLAGS) play_state.c

util.o: util.c
	$(CC) $(CFLAGS) util.c

timer.o: timer.c
	$(CC) $(CFLAGS) timer.c

menu_state.o: menu_state.c
	$(CC) $(CFLAGS) menu_state.c

level_state.o: level_state.c
	$(CC) $(CFLAGS) level_state.c

pause_state.o: pause_state.c
	$(CC) $(CFLAGS) pause_state.c

game_over_state.o: game_over_state.c
	$(CC) $(CFLAGS) game_over_state.c

bonus.o: bonus.c
	$(CC) $(CFLAGS) bonus.c

EDITOR_EXE: $(EDITOR_OBJS)
	$(CC) $(LDFLAGS) $(EDITOR_OBJS) -o $(EDITOR_EXE)

src/editor/main.o: src/editor/main.c
	$(CC) $(CFLAGS) src/editor/main.c -o src/editor/main.o

.PHONY: clean
clean:
	rm -f $(EXE_RELEASE) $(EXE_DBG) $(EDITOR_EXE) src/*.o src/editor/*.o cscope.in.out cscope.out cscope.po.out tags

.PHONY: tags
tags:
	ctags -R
	cscope -bqR

