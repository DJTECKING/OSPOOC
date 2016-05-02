CC		= gcc
IFLAGS	= -Iinclude
CFLAGS	= -std=c99 -fPIC -pedantic -Wall -Wextra -c # -march=native -ggdb3
LFLAGS	= -shared -lX11 -lXext # -march=native -ggdb3
DFLAGS	= -L./lib -lOSPOOC

TARGET	= lib/libOSPOOC.so
SOURCES	= $(shell echo src/*.c)
HEADERS	= $(shell echo include/*.h)
OBJECTS	= $(SOURCES:.c=.o)

# CURLIBPATH = $(PWD)/lib
# LDLIBPATH = $(shell echo $(LD_LIBRARY_PATH) | grep $(CURLIBPATH))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJECTS)

*.o: *.c
	$(CC) $(IFLAGS) $(CFLAGS) -o $@ $<

demo: $(TARGET) demo/windows.c
	$(CC) $(IFLAGS) -o bin/windows demo/windows.c $(DFLAGS)

clean:
	rm -r */*.o */*.so bin/*

