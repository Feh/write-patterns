#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

struct bench_func {
	ssize_t (*f)(int, int);
	char *desc;
};

void time_run(struct bench_func *arg, int in, int out)
{
	int i;
	struct timespec start, end;
	double delta;

	assert(arg != NULL);

	fprintf(stderr, "%-30s", arg->desc);

	for(i = 0; i < 3; i++) {
		lseek(in, 0, SEEK_SET);
		lseek(out, 0, SEEK_SET);
		ftruncate(out, 0);

		clock_gettime(CLOCK_MONOTONIC, &start);
		arg->f(in, out);
		clock_gettime(CLOCK_MONOTONIC, &end);

		delta = 1000 * (end.tv_sec - start.tv_sec);
		delta += (end.tv_nsec - start.tv_nsec) / 100000.0;
		fprintf(stderr, " % 12.2fms", delta);
	}

	fprintf(stderr, "\n");
}

int is_regular_file(int fd)
{
	struct stat st;
	if(fstat(fd, &st) == -1)
		return -1;
	return (S_ISREG(st.st_mode));
}

ssize_t block_size(int fd)
{
	struct statfs st;
	assert(fstatfs(fd, &st) != -1);
	return (ssize_t) st.f_bsize;
}

ssize_t filesize(int fd)
{
	struct stat st;
	assert(fstat(fd, &st) != -1);
	return (ssize_t) st.st_size;
}

ssize_t dummy(int in, int out)
{
	ssize_t i, j = 0;
	for(i = 0; i < 1000000; i++)
		j += i * i * i;
	return (ssize_t) j;
}

ssize_t read_write_1k(int in, int out)
{
	ssize_t w = 0, r = 0, t, n, m;
	ssize_t blocksize = 1024;

	char *buf = malloc(blocksize);
	assert(buf != NULL);

	t = filesize(in);

	while(r < t && (n = read(in, buf, blocksize))) {
		if(n == -1) { assert(errno == EINTR); continue; }
		r += n;
		while(w < r && (m = write(out, buf, (r - w)))) {
			if(m == -1) { fprintf(stderr, "%m\n");assert(errno == EINTR); continue; }
			w += m;
		}
	}
	return w;
}

int main(int argc, char *argv[])
{
	int i;
	int in = STDIN_FILENO, out = STDOUT_FILENO;
	struct bench_func fs[] = {
		{ dummy, "dummy" },
		{ read_write_1k, "read+write 1k" },
	};

	if(!is_regular_file(in) || !is_regular_file(out)) {
		assert(argc >= 1);
		fprintf(stderr, "usage: %s <in >out\n", argv[0]);
		exit(-1);
	}

	for(i = 0; i < sizeof(fs)/sizeof(fs[0]); i++)
		time_run(&fs[i], in, out);

	return 0;
}

/* vim:set sw=8 ts=8 noet: */
