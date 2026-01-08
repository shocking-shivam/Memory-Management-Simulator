CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -g -I.
SRC = main.c my_sbrk.c \
      allocator/allocator.c allocator/buddy.c allocator/first_fit.c allocator/best_fit.c allocator/worst_fit.c \
      cache/cache.c observability/memory_dump.c simulator/cli.c stats/stats.c
OBJ = $(SRC:.c=.o)
TARGET = memsim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
