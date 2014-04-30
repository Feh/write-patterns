#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/resource.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

struct bench_func {
	ssize_t (*f)(int, int);
	char *desc;
	int t[3];
};

int cmp_int(const void *a, const void *b);
int cmp_bench_func(const void *a, const void *b);
int is_regular_file(int fd);
ssize_t block_size(int fd);
ssize_t do_sendfile(int in, int out);
ssize_t dummy_read_file(int in, int out);
ssize_t filesize(int fd);
ssize_t mmap_write_advices_falloc(int in, int out);
ssize_t mmap_write_advices(int in, int out);
ssize_t mmap_write_advices_truncate(int in, int out);
ssize_t mmap_write(int in, int out);
ssize_t pipe_splice_advices_falloc(int in, int out);
ssize_t pipe_splice_advices(int in, int out);
ssize_t pipe_splice_advices_truncate(int in, int out);
ssize_t pipe_splice(int in, int out);
void print_benchfunc(struct bench_func *arg, const char *eol, int ref);
ssize_t read_write_16bs_advices_falloc(int in, int out);
ssize_t read_write_16bs_advices(int in, int out);
ssize_t read_write_16bs_advices_truncate(int in, int out);
ssize_t read_write_16xopt(int in, int out);
ssize_t read_write_1k(int in, int out);
ssize_t read_write_256xopt(int in, int out);
ssize_t read_write_4xopt(int in, int out);
ssize_t read_write_bs(int in, int out, ssize_t bs);
ssize_t read_write_opt_fact(int in, int out, ssize_t fact);
ssize_t read_write_opt(int in, int out);
ssize_t sendfile_advices_falloc(int in, int out);
ssize_t sendfile_advices(int in, int out);
ssize_t sendfile_advices_truncate(int in, int out);
void advice(int in, int out);
void do_falloc(int in, int out);
void enlarge_truncate(int in, int out);
void time_run(struct bench_func *arg, int in, int out);


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

int cmp_int(const void *a, const void *b)
{
	return ((*(int *)a) < (*(int *)b) ? -1 : 1);
}

int cmp_bench_func(const void *a, const void *b)
{
	return ((((struct bench_func *)a)->t[0]) < (((struct bench_func *)b)->t[0]) ? -1 : 1);
}

void time_run(struct bench_func *arg, int in, int out)
{
	int i;
	struct timespec start, end;

	assert(arg != NULL);

	for(i = 0; i < sizeof(arg->t)/sizeof(arg->t[0]); i++) {
		lseek(in, 0, SEEK_SET);
		lseek(out, 0, SEEK_SET);
		ftruncate(out, 0);
		usleep(100000);

		clock_gettime(CLOCK_MONOTONIC, &start);
		arg->f(in, out);
		clock_gettime(CLOCK_MONOTONIC, &end);

		assert(arg->f == dummy_read_file || filesize(in) == filesize(out));

		arg->t[i] = 1000 * (end.tv_sec - start.tv_sec);
		arg->t[i] += (end.tv_nsec - start.tv_nsec) / 1000000;
	}

	qsort(arg->t, sizeof(arg->t)/sizeof(arg->t[0]), sizeof(arg->t[0]), cmp_int);
	print_benchfunc(arg, "\r", 0);
}


void print_benchfunc(struct bench_func *arg, const char *eol, int ref)
{
	int i;
	fprintf(stderr, "%-40s", arg->desc);
	for(i = 0; i < sizeof(arg->t)/sizeof(arg->t[0]); i++)
		fprintf(stderr, " % 8dms", arg->t[i]);
	if(ref > 0)
		fprintf(stderr, "  (+% 2.1f%%)", (100.0 * (arg->t[0] - ref)) / ref);
	fprintf(stderr, eol ? eol : "\n");
}

ssize_t dummy_read_file(int in, int out)
{
	ssize_t n;
	char buf[8192];

	while((n = read(in, buf, sizeof(buf)))) {
		if(n == -1) { assert(errno == EINTR); continue; }
		/* do nothing */
	}

	return 0;
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

	free(buf);

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

ssize_t pipe_splice(int in, int out)
{
	size_t bs = 65536;
	ssize_t w = 0, r = 0, t, n, m;
	int pipefd[2];
	int flags = SPLICE_F_MOVE | SPLICE_F_MORE;

	assert(pipe(pipefd) != -1);

	t = filesize(in);

	while(r < t && (n = splice(in, NULL, pipefd[1], NULL, bs, flags))) {
		if(n == -1) { assert(errno == EINTR); continue; }
		r += n;
		while(w < r && (m = splice(pipefd[0], NULL, out, NULL, bs, flags))) {
			if(m == -1) { assert(errno == EINTR); continue; }
			w += m;
		}
	}

	close(pipefd[0]);
	close(pipefd[1]);

	return w;
}

ssize_t do_sendfile(int in, int out)
{
	ssize_t t = filesize(in);
	off_t ofs = 0;
	while(ofs < t) {
		if(sendfile(out, in, &ofs, t - ofs) == -1) {
			assert(errno == EINTR);
			continue;
		}
	}
	return t;
}

void advice(int in, int out)
{
	ssize_t t = filesize(in);
	posix_fadvise(in, 0, t, POSIX_FADV_WILLNEED);
	posix_fadvise(in, 0, t, POSIX_FADV_SEQUENTIAL);
}

void do_falloc(int in, int out)
{
	ssize_t t = filesize(in);
	posix_fallocate(out, 0, t);
}

void enlarge_truncate(int in, int out)
{
	ssize_t t = filesize(in);
	ftruncate(out, t);
}

ssize_t read_write_16bs_advices(int in, int out)
{
	advice(in, out);
	return read_write_16xopt(in, out);
}

ssize_t read_write_16bs_advices_falloc(int in, int out)
{
	advice(in, out);
	do_falloc(in, out);
	return read_write_16xopt(in, out);
}

ssize_t read_write_16bs_advices_truncate(int in, int out)
{
	advice(in, out);
	enlarge_truncate(in, out);
	return read_write_16xopt(in, out);
}

ssize_t mmap_write_advices(int in, int out)
{
	advice(in, out);
	return mmap_write(in, out);
}

ssize_t mmap_write_advices_falloc(int in, int out)
{
	advice(in, out);
	do_falloc(in, out);
	return mmap_write(in, out);
}

ssize_t mmap_write_advices_truncate(int in, int out)
{
	advice(in, out);
	enlarge_truncate(in, out);
	return mmap_write(in, out);
}

ssize_t pipe_splice_advices(int in, int out)
{
	advice(in, out);
	return pipe_splice(in, out);
}

ssize_t pipe_splice_advices_falloc(int in, int out)
{
	advice(in, out);
	do_falloc(in, out);
	return pipe_splice(in, out);
}

ssize_t pipe_splice_advices_truncate(int in, int out)
{
	advice(in, out);
	enlarge_truncate(in, out);
	return pipe_splice(in, out);
}

ssize_t sendfile_advices(int in, int out)
{
	advice(in, out);
	return do_sendfile(in, out);
}

ssize_t sendfile_advices_falloc(int in, int out)
{
	advice(in, out);
	do_falloc(in, out);
	return do_sendfile(in, out);
}

ssize_t sendfile_advices_truncate(int in, int out)
{
	advice(in, out);
	enlarge_truncate(in, out);
	return do_sendfile(in, out);
}

int main(int argc, char *argv[])
{
	int i;
	int in = STDIN_FILENO, out = STDOUT_FILENO;
	struct bench_func fs[] = {
		{ read_write_1k, "read+write 1k" },
		{ read_write_opt, "read+write bs" },
		{ read_write_4xopt, "read+write 4bs" },
		{ read_write_16xopt, "read+write 16bs" },
		{ read_write_16bs_advices, "read+write 16bs + advices" },
		{ read_write_16bs_advices_falloc, "read+write 16bs + advices + falloc" },
		{ read_write_16bs_advices_truncate, "read+write 16bs + advices + trunc" },
		{ read_write_256xopt, "read+write 256bs" },
		{ mmap_write, "mmap+write" },
		{ mmap_write_advices, "mmap+write + advices" },
		{ mmap_write_advices_falloc, "mmap+write + advices + falloc" },
		{ mmap_write_advices_truncate, "mmap+write + advices + trunc" },
		{ pipe_splice, "pipe+splice" },
		{ pipe_splice_advices, "pipe+splice + advices" },
		{ pipe_splice_advices_falloc, "pipe+splice + advices + falloc" },
		{ pipe_splice_advices_truncate, "pipe+splice + advices + trunc" },
		{ do_sendfile, "sendfile" },
		{ sendfile_advices, "sendfile + advices" },
		{ sendfile_advices_falloc, "sendfile + advices + falloc" },
		{ sendfile_advices_truncate, "sendfile + advices + trunc" },
	};

	if(!is_regular_file(in) || !is_regular_file(out)) {
		assert(argc >= 1);
		fprintf(stderr, "usage: %s <in >out\n", argv[0]);
		exit(-1);
	}

	dummy_read_file(in, out); /* don’t give anybody a head start... */

	for(i = 0; i < sizeof(fs)/sizeof(fs[0]); i++)
		time_run(&fs[i], in, out);

	qsort(fs, sizeof(fs)/sizeof(fs[0]), sizeof(fs[0]), cmp_bench_func);
	for(i = 0; i < sizeof(fs)/sizeof(fs[0]); i++)
		print_benchfunc(&fs[i], "\n", i == 0 ? 0 : fs[0].t[0]);

	return 0;
}

/* vim:set sw=8 ts=8 noet: */
