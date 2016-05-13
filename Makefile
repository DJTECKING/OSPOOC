CC		= gcc -g
IFLAGS	= -Iinclude
CFLAGS	= -std=c99 -fPIC -pedantic -Wall -Wextra -c
DFLAGS	= -DOSP_XDBE_SUPPORT -D_XOPEN_SOURCE
LFLAGS	= -shared -lX11 -lXext -lm
EFLAGS	= -L./lib -lOSPOOC

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
	$(CC) $(IFLAGS) $(CFLAGS) -o $@ $< $(DFLAGS)

demo: $(TARGET) demo/windows.c
	$(CC) $(IFLAGS) -o bin/windows demo/windows.c $(EFLAGS) -lm -DOSP_XDBE_SUPPORT

clean:
	rm -r */*.o */*.so bin/*

