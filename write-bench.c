#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>

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
	struct timespec start, end;
	double delta;

	assert(arg != NULL);

	clock_gettime(CLOCK_MONOTONIC, &start);
	arg->f(STDIN_FILENO, STDOUT_FILENO);

	clock_gettime(CLOCK_MONOTONIC, &end);

	delta = 1000 * (end.tv_sec - start.tv_sec);
	delta += (end.tv_nsec - start.tv_nsec) / 100000.0;

	fprintf(stdout, "%-40s %.2fms\n", arg->desc, delta);
}

int main(int argc, char *argv[])
{
	int i;
	struct bench_func fs[] = {
		{ dummy, "dummy" },
	};

	for(i = 0; i < sizeof(fs)/sizeof(fs[0]); i++)
		time_run(&fs[i]);

	return 0;
}

/* vim:set sw=8 ts=8 noet: */
