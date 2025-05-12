CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I. -pthread -D_GNU_SOURCE
SRC = main.c mongoose/mongoose.c input/input.c
OBJ = $(SRC:.c=.o)
EXEC = server

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
