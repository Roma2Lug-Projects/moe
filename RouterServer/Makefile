CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O3

all: $(OBJ)
	${CC} ${CFLAGS} *.c -o main.elf 

.PHONY: clean

clean:
	@rm -f *.elf *.bin *.o *.img
