CC=gcc
CFLAGS=-Wall -Wextra -O2
CHEADERS=$(shell ls *.h)
CFILES=$(shell ls *.c)
LIBS=-pthread -lsqlite3
TARGET=moe-receiver

all: $(TARGET)
	

$(TARGET): $(CFILES) $(CHEADERS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

