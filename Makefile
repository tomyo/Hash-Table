CC=gcc
CFLAGS+= -Wall -Wextra -Wdeclaration-after-statement -Wbad-function-cast -Wcast-qual -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wno-unused-parameter -g -ansi # -Werror
SOURCES=$(shell echo *.c)
OBJECTS= $(SOURCES:.c=.o)

all: $(OBJECTS)
#	$(CC) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) .depend
	#make -C tests clean

test: $(OBJECTS)
	make -C tests test

memtest: $(OBJECTS)
	make -C tests memtest

docs:
	doxygen Doxyfile

.depend: *.[ch]
	$(CC) -MM $(SOURCES) >.depend

-include .depend

.PHONY: clean all
