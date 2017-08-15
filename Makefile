CC=gcc -g
CFLAGS=-Wall -Wextra
TARGET=main
LIBS=cJSON/cJSON.c cJSON/cJSON.h

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
clean:
	rm $(TARGET) 

