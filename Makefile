CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g
TARGET=main
LIBS=cJSON/cJSON.c cJSON/cJSON.h

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
clean:
	rm $(TARGET) 

