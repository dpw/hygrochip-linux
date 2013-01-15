CFLAGS=-Wall

hyt-read: hyt-read.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm hyt-read
