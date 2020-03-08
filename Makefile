TARGET = vmiss
COMPILER = gcc
OBJECTS = $(SOURCES:%.c=%.o)
SOURCES = disassm.c interpreter.c json_rsc.c util.c vmiss.c x86emu.c main.c

all: $(TARGET)

.SUFFIXES: .c .o
.c.o:
	$(COMPILER) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(COMPILER) -o $@ $(OBJECTS) 

test6c:
	m2cc test/6.c
	./vmiss a.out > out/test6c.log 2>&1

clean:
	rm -f *.o
	rm -rf out/*
	rm -f vmiss

