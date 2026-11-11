CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
SRC = $(wildcard src/*.c)
HDR = $(wildcard include/*.h)
RM = rm -f

ifeq ($(OS), Windows_NT)
    #Windows 
    EXEC    = ibat.exe
    RAYLIB  = C:/raylib/raylib/src
    CFLAGS += -I$(RAYLIB)
    LIBS    = -L$(RAYLIB) -lraylib -lopengl32 -lgdi32 -lwinmm
else
    #Linux / WSL
    EXEC    = ibat
    LIBS    = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

.PHONY: all run clean

all: $(EXEC)

$(EXEC): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

run: $(EXEC)
	./$(EXEC)

clean:
	$(RM) $(EXEC)