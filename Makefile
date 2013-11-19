CFLAGS=-Wall
prefix:=/usr/local

hyt-read: hyt-read.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: install
install: hyt-read
	mkdir -p $(prefix)/sbin
	install -m 0755 -t $(prefix)/sbin $^

.PHONY: uninstall
uninstall:
	rm -f $(prefix)/sbin/hyt-read

.PHONY: clean
clean:
	rm -f hyt-read
