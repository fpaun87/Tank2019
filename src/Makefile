include ../Makefile

#SHARE_PATH := $(TOP)/Shared/
CFLAGS += -I /usr/local/include
LDFLAGS += -L /usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
EXE_LOCATION:= ..
EXE_DBG := Tank2019_dbg
EXE_RELEASE := Tank2019

ifeq ($(debug),1)
EXE := $(EXE_DBG)
else
EXE := $(EXE_RELEASE)
endif

SRC := main.c resource_mgr.c play_state.c util.c menu_state.c level_state.c pause_state.c game_over_state.c timer.c

OBJS:=$(SRC:.c=.o)

EXE: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXE_LOCATION)/$(EXE)

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

.PHONY: clean
clean:
	rm -f $(EXE_LOCATION)/$(EXE_RELEASE) $(EXE_LOCATION)/$(EXE_DBG) *.o
