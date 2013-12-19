CC=gcc
CFLAGS=-Wall -Wextra -O2
CHEADERS=$(shell ls *.h)
CFILES=$(shell ls *.c)
LIBS=
TARGET=moe-sender

all: $(TARGET)
	

$(TARGET): $(CFILES) $(CHEADERS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

