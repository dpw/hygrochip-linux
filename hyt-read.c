#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <stdarg.h>

#include <linux/i2c-dev.h>

static void die_errno(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, ": %s\n", strerror(errno));
	exit(1);
}

static void die_alloc()
{
	fprintf(stderr, "failed to allocate memory\n");
	exit(1);
}

/* Does the file at <dir>/<subdir>/name contain the given i2c bus name? */
static int name_file_matches(const char *dir, const char *subdir,
			     const char *want)
{
	char *path;
	char *name;
	FILE *fp;
	int res;

	if (asprintf(&path, "%s/%s/name", dir, subdir) < 2)
		die_alloc();

	fp = fopen(path, "r");
	if (fp == NULL)
		die_errno("opening %s", path);

	if (fscanf(fp, "%as", &name) == 1) {
		res = (strcmp(name, want) == 0);
		free(name);
	}
	else {
		if (ferror(fp))
			die_errno("reading %s", path);

		/* contents of file were odd */
		res = 0;
	}

	fclose(fp);
	free(path);
	return res;
}

/* Open /dev/blah */
static int open_i2c_dev(const char *file)
{
	char *path;
	int fd;

	if (asprintf(&path, "/dev/%s", file) < 1)
		die_alloc();

	fd = open(path, O_RDWR);
	if (fd < 0)
		die_errno("opening %s", path);

	free(path);
	return fd;
}

/* Find the i2c bus with the given name, and open it. */
static int open_i2c_bus(const char *name)
{
	const char *dir = "/sys/class/i2c-dev";
	struct dirent *de;
	DIR *dp = opendir(dir);
	if (!dp)
		die_errno("opening directory %s", dir);

	for (;;) {
		errno = 0;
		de = readdir(dp);
		if (!de) {
			if (errno)
				die_errno("reading directory %s", dir);

			break;
		}

		if (de->d_name[0] == '.')
			continue;

		if (name_file_matches(dir, de->d_name, name)) {
			int fd = open_i2c_dev(de->d_name);
			closedir(dp);
			return fd;
		}
	}

	fprintf(stderr, "could not find i2c bus %s\n", name);
	exit(1);
}

struct reading {
	float humidity;
	float temperature;
};

static void take_reading(int fd, struct reading *r)
{
	unsigned char data[4];
	ssize_t sz;

	data[0] = 0;
	sz = write(fd, data, 1);
	if (sz < 0)
		die_errno("writing to i2c");

	/* wait for sensor to provide reading */
	usleep(60 * 1000);

	sz = read(fd, data, 4);
	if (sz < 0)
		die_errno("reading from i2c");

	if (sz < 4) {
		fprintf(stderr, "short read (%d bytes)\n", (int)sz);
		exit(1);
	}

	/*printf("%x %x %x %x\n", data[0], data[1], data[2], data[3]);*/

	/*
	 * Sensor reading are two bytes for humidity, and two bytes
	 * for temperature, big-endian.  The top two bits of the
	 * humidity value and the bottom two bits of the temperature
	 * value are status bits (of undocumented purpose).  Humidity
	 * readings range from 0 to 100%; temperature readings range
	 * from -40 to 120 degrees C.  In both cases, the ranges
	 * correspond to the full range of available bits.
	 */
	r->humidity = ((data[0] & 0x3f) << 8 | data[1]) * (100.0 / 0x3fff);
	r->temperature = (data[2] << 8 | (data[3] & 0xfc)) * (165.0 / 0xfffc) - 40;
}

void usage()
{
	printf("Usage: hyt-read [ -i seconds ] [ -T | -H ] [ -b i2c bus name | device file ]\n"
		"\t-b X\tOpen the device named X\n"
		"\t-i X\tRead data every X seconds\n"
		"\t-T\tPrint only temperature\n"
		"\t-H\tPrint only humidity\n"
		"\t-h\tShow this message\n\n"
	);
}

int main(int argc, char **argv)
{
	int fd = 0, c, interval = 0;
	short ptemp = 1, phum = 1;
	unsigned int slave = 0x28;

	while ((c = getopt (argc, argv, "HTb:i:h")) != -1) {
		switch (c) {
			case 'b':
				fd = open_i2c_bus(optarg);
			break;

			case 'i':
				interval=atoi(optarg);
			break;

			case 'T':
				phum=0;
				ptemp=1;
			break;

			case 'H':
				ptemp=0;
				phum=1;
			break;

			case 'h':
				usage();
				return 1;
			break;
		}
	}
	if ((argc-optind)<1){
		usage();
		return 1;
	}
	/* If the bus name was not specified, argv[optind] is the device file */
	if (!fd){
		fd=open_i2c_dev(argv[optind]);
		optind++;
	}

	if (argc == optind+1) {
		char *endptr;
		long n = strtol(argv[optind], &endptr, 0);

		if (endptr == argv[optind] || *endptr != 0) {
			fprintf(stderr, "bad slave address '%s'\n", argv[optind]);
			return 1;
		}

		if (n < 0x3 || n > 0x77) {
			fprintf(stderr, "slave address %ld out of range\n", n);
			return 1;
		}

		slave = n;
	}

	if (ioctl(fd, I2C_SLAVE, slave) < 0)
		die_errno("ioctl(I2C_SLAVE)");

	/* initiate reading */
	do {
		char *sep = "";
		struct reading r;
		take_reading(fd, &r);

		if (phum) {
			printf("%f", r.humidity);
			sep = " ";
		}

		if (ptemp) {
			printf("%s%f", sep, r.temperature);
		}

		printf("\n");

		sleep(interval);
	} while (interval > 0);

	close(fd);
	return 0;
}
