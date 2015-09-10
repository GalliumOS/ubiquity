/*
 * Copyright (C) 2006 David Härdeman <david@2gen.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h> /* for BLKGETSIZE definitions */
#include <getopt.h>
#include <string.h>
#include <time.h>

/* Progress indicator to output */
# define PROGRESS_INDICATOR "*\n"
/* How many progress indicators to output in total */
#define PROGRESS_PARTS 1000
/* How many bytes to write at a time (default) */
#define DEFAULT_WSIZE 4096
/* Debugging output */
#ifdef DEBUG
#define dprintf(x, ...) fprintf(stderr, x, ...)
#else
#define dprintf(x, ...)
#endif

static void die(const char *message, int perr)
{
	if (perr)
		perror(message);
	else
		fprintf(stderr, "Error: %s\n", message);
	exit(1);
}

static void usage(const char *message, const char *argv0)
{
	printf("%s: %s\n"
	       "Usage: %s [options] device\n"
	       "  -f=FILE\tread data to write from file instead of writing zero\n"
	       "  -s=NUM\twrite NUM bytes at a time (default %i)\n"
	       , argv0, message, argv0, DEFAULT_WSIZE);
	exit(1);
}

static unsigned long long dev_size(int fd)
{
	int ret;
	unsigned long long size;

	ret = ioctl(fd, BLKGETSIZE64, &size);
	if (ret < 0) {
		close(fd);
		die("failed to get device size", 1);
	}

	return size;
}

static int do_wipe(int source, int target, size_t wsize)
{
	unsigned long long size;
	unsigned long long done = 0;
	unsigned int previous_progress = 0;
	unsigned int progress = 0;
	time_t last_sync = 0;
	int i;
	ssize_t count;
	char *buf = calloc(wsize, 1);

	if (buf == NULL)
		die("buffer allocation failed", 1);

	size = dev_size(target);
	dprintf("Block size in bytes is %llu\n", size);
	/* From now on, try to make sure stdout is unbuffered */
	setbuf(stdout, NULL);

	while (done < size) {
		/* First read (if we have a source) */
		if (source) {
			count = read(source, buf, wsize);
			if (count != wsize)
				die("failed to read from source", 1);
		}

		/* Now write what we just read */
		count = write(target, buf, wsize);
		dprintf("Count is %zi\n", count);
		if (count < 0)
			die("failed to write to target", 1);

		/* Calculate progress */
		done += count;
		progress = ((done * PROGRESS_PARTS)/ size);
		dprintf("We just wrote %zi, done %llu\n", count, done);
		if (progress > previous_progress) {
			/* sync on every progress output, but at most once per second */
			time_t now = time(NULL);
			if (now != last_sync) {
				fdatasync(target);
				last_sync = now;
			}
			for (i = 0; i < progress - previous_progress; i++)
				printf(PROGRESS_INDICATOR);
			previous_progress = progress;
		}
	}

	free(buf);
	fdatasync(target);
	return 0;
}

int main(int argc, char **argv, char **envp)
{
	int target, source, ret;
	char *sourcefname = NULL;
	size_t wsize = DEFAULT_WSIZE;

	/* Parse options */
	while (1) {
		ret = getopt(argc, argv, "f:s:");
		if (ret == -1)
			break;
		switch(ret) {
			case 'f':
				sourcefname = optarg;
				break;
			case 's':
				wsize = (size_t)atol(optarg);
				if (wsize < 1)
					die("incorrect size", 0);
				break;
			default:
				die("unknown getopt failure", 0);
				break;
		}
	}
	if (argc - optind != 1)
		usage("you must specify one target device", argv[0]);

	/* Get target */
	target = open(argv[optind], O_WRONLY);
	if (target < 0)
		die("failed to open device", 1);

	/* Get source */
	if (sourcefname) {
		source = open(sourcefname, O_RDONLY);
		if (source < 0)
			die("failed to open source", 1);
	} else {
		source = 0;
	}

	/* Wipe device */
	ret = do_wipe(source, target, wsize);

	/* Clean up */
	close(source);
	close(target);
	if (ret)
		die("failed to wipe device", 0);
	return 0;
}

