TOP := /home/florin/projects/c/tank2019
CFLAGS :=  -Wall -Wextra -fno-omit-frame-pointer -c
debug := 1

ifeq ($(debug),1)
	CFLAGS += -ggdb3 -pg -O0
else
	CFLAGS += -O3
endif

