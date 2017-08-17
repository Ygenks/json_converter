CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11
TARGET=main
LIBS=cJSON/cJSON.c cJSON/cJSON.h

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
clean:
	rm $(TARGET) 

