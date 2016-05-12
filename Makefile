CC		= gcc -g
IFLAGS	= -Iinclude
CFLAGS	= -std=c99 -fPIC -pedantic -Wall -Wextra -c -D_XOPEN_SOURCE# -march=native -ggdb3
LFLAGS	= -shared -lX11 -lXext -lm -D_XOPEN_SOURCE# -march=native -ggdb3
DFLAGS	= -L./lib -lOSPOOC

TARGET	= lib/libOSPOOC.so
SOURCES	= $(shell echo src/*.c)
HEADERS	= $(shell echo include/*.h)
OBJECTS	= $(patsubst %.c,%.o, $(SOURCES))

# CURLIBPATH = $(PWD)/lib
# LDLIBPATH = $(shell echo $(LD_LIBRARY_PATH) | grep $(CURLIBPATH))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(IFLAGS) $(CFLAGS) -o $@ $<

demo: $(TARGET) demo/windows.c
	$(CC) $(IFLAGS) -o bin/windows demo/windows.c $(DFLAGS) -lm

clean:
	rm -r */*.o */*.so bin/*

