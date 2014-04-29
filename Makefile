write-bench: write-bench.c
	gcc -O2 -Wall -Werror -Wstack-protector $< -o $@
clean:
	rm -f write-bench
