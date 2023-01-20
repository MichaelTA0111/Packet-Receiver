# The compiler to use:
CC = cc

# Compiler flags:
CFLAGS = -g -O2 -Wall

# Files to link:
LFLAGS = consumer.c

# The output file:
OUTPUT = application.o

# The target source file
TARGET = main.c

all: $(OUTPUT)

$(OUTPUT): $(TARGET)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(TARGET) $(LFLAGS)

clean:
	rm -f $(OUTPUT)

