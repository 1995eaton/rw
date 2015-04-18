C=gcc
CFLAGS=-c -std=gnu99 -O2 -Wall -Werror -Wextra -pedantic
SRC=util.c rw.c main.c
OBJ=$(SRC:.c=.o)
TARGET=rw
PREFIX=/usr/local

all: $(SRC) $(TARGET)

$(TARGET): $(OBJ)
	$(C) $(OBJ) -o $@

.c.o:
	$(C) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) $(PREFIX)/bin
