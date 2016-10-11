EXE_NAME = chip
OBJS = CPU.o main.o
CC = gcc
FLAGS = -Wall -O2
LIBS = -lSDL2

.c.o:
	$(CC) $< -c $(FLAGS) $(INC)

$(EXE_NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(FLAGS) $(LIBS)
clean:
	rm -f $(OBJS) $(EXE_NAME)
