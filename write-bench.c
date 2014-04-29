#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

struct bench_func {
	ssize_t (*f)(int, int);
	char *desc;
};

ssize_t dummy(int in, int out)
{
	ssize_t i, j = 0;
	for(i = 0; i < 1000000; i++)
		j += i * i * i;
	return (ssize_t) j;
}

void time_run(struct bench_func *arg)
{
	int i;
	struct timespec start, end;
	double delta;

	assert(arg != NULL);

	fprintf(stderr, "%-30s", arg->desc);

	for(i = 0; i < 3; i++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		arg->f(STDIN_FILENO, STDOUT_FILENO);
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

int main(int argc, char *argv[])
{
	int i;
	struct bench_func fs[] = {
		{ dummy, "dummy" },
	};

	if(!is_regular_file(STDIN_FILENO) || !is_regular_file(STDOUT_FILENO)) {
		fprintf(stderr, "usage: %s <in >out\n", argv[0]);
		exit(-1);
	}

	for(i = 0; i < sizeof(fs)/sizeof(fs[0]); i++)
		time_run(&fs[i]);

	return 0;
}

/* vim:set sw=8 ts=8 noet: */
