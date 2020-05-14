CC=gcc
CFLAGS=-c -Wall -DSOURCES_VERSION=\"0.0.0-0\"
LDFLAGS=-pthread
SOURCES=data.c  dog.c  helpers.c  mongoose.c  mongoose_helper.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dog

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -f dog *.o
