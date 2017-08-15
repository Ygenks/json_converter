CC=gcc
TARGET=main

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^
clean:
	rm $(TARGET) 

