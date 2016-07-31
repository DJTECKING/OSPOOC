CC		= gcc -g
IFLAGS	= -Iinclude
CFLAGS	= -std=c99 -fPIC -pedantic -Wall -Wextra -c
EXTFLS	= -DOSP_XDBE_SUPPORT
DFLAGS	= -D_XOPEN_SOURCE $(EXTFLS)
LFLAGS	= -shared -lX11 -lXext -lm -lGL
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

demo: $(TARGET) demo/ball.c demo/cube.c demo/banana_crush_saga/banana_crush_saga.c
	$(CC) $(IFLAGS) -o bin/ball demo/ball.c $(EFLAGS) -lm $(EXTFLS)
	$(CC) $(IFLAGS) -o bin/cube demo/cube.c $(EFLAGS) -lm -lGL
	$(CC) $(IFLAGS) -o bin/banana_crush_saga demo/banana_crush_saga/banana_crush_saga.c $(EFLAGS) -lm $(EXTFLS)

clean:
	rm -r */*.o */*.so bin/*

