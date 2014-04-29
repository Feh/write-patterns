write-bench: write-bench.c
	gcc -O2 -Wall -Werror -Wstack-protector -lrt $< -o $@
clean:
	rm -f write-bench
