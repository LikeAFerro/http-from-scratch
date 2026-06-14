CC = gcc
CFLAGS = -g -Wall -Wextra -O2
# AddressSanitizer flags for memory debugging
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

TARGET = http-from-scratch
SRC = src/http-from-scratch.c src/assets.c
# Convert .c source files into .o object files (keeps the src/ prefix)
OBJ = $(SRC:.c=.o)

all: $(TARGET)

# Compile .c files into .o files (handles src/ paths perfectly)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

test: all
	./test.sh

# Force a clean environment, compile with ASAN, test, and clean up afterwards
asan:
	$(MAKE) clean
	$(MAKE) $(TARGET) CFLAGS="$(CFLAGS) $(ASAN_FLAGS)"
	./test.sh
	$(MAKE) clean

valgrind: all
	./test.sh valgrind

clean:
	rm -f $(TARGET) $(OBJ) test.log

.PHONY: all test clean asan valgrind
