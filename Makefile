write-bench: write-bench.c Makefile
	gcc -O2 -Wall -Werror -Wstack-protector -Wmissing-prototypes -lrt $< -o $@
clean:
	rm -f write-bench
