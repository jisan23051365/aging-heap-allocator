# Makefile — Aging Heap Allocator

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

TARGET  = aging_heap_demo
SRCS    = aging_heap.c main.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c aging_heap.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(TARGET)
