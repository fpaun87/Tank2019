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

SRC := main.c resource_mgr.c play_state.c util.c menu_state.c

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

menu_state.o: menu_state.c
	$(CC) $(CFLAGS) menu_state.c


.PHONY: clean
clean:
	rm -f $(EXE_LOCATION)/$(EXE_RELEASE) $(EXE_LOCATION)/$(EXE_DBG) *.o
