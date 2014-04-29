#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/mman.h>
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

int cmp_dbl(const void *a, const void *b)
{
	return ((*(double *)a) < (*(double *)b) ? -1 : 1);
}

void time_run(struct bench_func *arg, int in, int out)
{
	int i;
	struct timespec start, end;
	double t[3];

	assert(arg != NULL);

	for(i = 0; i < sizeof(t)/sizeof(t[0]); i++) {
		lseek(in, 0, SEEK_SET);
		lseek(out, 0, SEEK_SET);
		ftruncate(out, 0);

		clock_gettime(CLOCK_MONOTONIC, &start);
		arg->f(in, out);
		clock_gettime(CLOCK_MONOTONIC, &end);

		t[i] = 1000 * (end.tv_sec - start.tv_sec);
		t[i] += (end.tv_nsec - start.tv_nsec) / 1000000.0;
	}

	qsort(t, sizeof(t)/sizeof(t[0]), sizeof(t[0]), cmp_dbl);
	fprintf(stderr, "%-30s", arg->desc);
	for(i = 0; i < sizeof(t)/sizeof(t[0]); i++)
		fprintf(stderr, " % 12.2fms", t[i]);
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
	return (ssize_t) 0;
}

ssize_t read_write_bs(int in, int out, ssize_t bs)
{
	ssize_t w = 0, r = 0, t, n, m;

	char *buf = malloc(bs);
	assert(buf != NULL);

	t = filesize(in);

	while(r < t && (n = read(in, buf, bs))) {
		if(n == -1) { assert(errno == EINTR); continue; }
		r += n;
		while(w < r && (m = write(out, buf, (r - w)))) {
			if(m == -1) { assert(errno == EINTR); continue; }
			w += m;
		}
	}
	return w;
}

ssize_t read_write_1k(int in, int out)
{
	return read_write_bs(in, out, 1024);
}

ssize_t read_write_opt_fact(int in, int out, ssize_t fact)
{
	ssize_t bs = block_size(out);
	assert(bs >= 1024);
	return read_write_bs(in, out, fact*bs);
}

ssize_t read_write_opt(int in, int out)
{
	return read_write_opt_fact(in, out, 1);
}

ssize_t read_write_4xopt(int in, int out)
{
	return read_write_opt_fact(in, out, 4);
}

ssize_t read_write_16xopt(int in, int out)
{
	return read_write_opt_fact(in, out, 16);
}

ssize_t read_write_256xopt(int in, int out)
{
	return read_write_opt_fact(in, out, 256);
}

ssize_t mmap_write(int in, int out)
{
	ssize_t w = 0, n;
	size_t len;
	char *p;

	len = filesize(in);
	p = mmap(NULL, len, PROT_READ, MAP_SHARED, in, 0);
	assert(p != NULL);

	while(w < len && (n = write(out, p, (len - w)))) {
		if(n == -1) { assert(errno == EINTR); continue; }
		w += n;
	}

	munmap(p, len);

	return w;
}

int main(int argc, char *argv[])
{
	int i;
	int in = STDIN_FILENO, out = STDOUT_FILENO;
	struct bench_func fs[] = {
		{ dummy, "dummy" },
		{ read_write_1k, "read+write 1k" },
		{ read_write_opt, "read+write bs" },
		{ read_write_4xopt, "read+write 4bs" },
		{ read_write_16xopt, "read+write 16bs" },
		{ read_write_256xopt, "read+write 256bs" },
		{ mmap_write, "mmap+write" },
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
