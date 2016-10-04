EXE_NAME = chip
OBJS = CPU.o main.o
CC = gcc
FLAGS = -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2 -Wall -O2
LIBS = -lm

.c.o:
	$(CC) $< -c $(FLAGS) $(INC)

$(EXE_NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(FLAGS) $(LIBS)
clean:
	rm -f $(OBJS) $(EXE_NAME)
