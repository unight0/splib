
CC = gcc
CFLAGS = -g -I.

sp: examples/main.c splib.h
	$(CC) $(CFLAGS) examples/main.c -o $@

fuzz: examples/main.c splib.h
	AFL_CC=gcc afl-gcc-fast $(CFLAGS) examples/main.c -o sp-fuzz

fuzz-run: fuzz
	@trap 'kill $$(jobs -p) 2>/dev/null' EXIT; \
	for i in $$(seq 1 15); do \
		afl-fuzz -i in-fuzz -o findings-fuzz -x splib.dict -S secondary$$i ./sp-fuzz >/dev/null 2>&1 & \
	done; \
	afl-fuzz -i in-fuzz -o findings-fuzz -x splib.dict -M main ./sp-fuzz

fuzz-run-resume: fuzz
	@trap 'kill $$(jobs -p) 2>/dev/null' EXIT; \
	for i in $$(seq 1 15); do \
		afl-fuzz -i - -o findings-fuzz -x splib.dict -S secondary$$i ./sp-fuzz >/dev/null 2>&1 & \
	done; \
	afl-fuzz -i - -o findings-fuzz -x splib.dict -M main ./sp-fuzz

fuzz-showmap: fuzz
	afl-showmap -C -i findings-fuzz/main/queue -o coverage.map -- ./sp-fuzz
	@echo "Total unique edges: $$(wc -l < coverage.map)"

sp-cov: examples/main.c splib.h
	$(CC) -fprofile-arcs -ftest-coverage $(CFLAGS) examples/main.c -o sp-cov

fuzz-gcov: sp-cov
	@for f in findings-fuzz/main/queue/id:*; do \
		./sp-cov < "$$f" >/dev/null 2>&1 || true; \
	done
	gcov sp-cov-main
	@echo "Coverage report: main.c.gcov, splib.h.gcov"
	@echo "Lines marked '#####' are unreached"

clean:
	rm -f sp sp-fuzz sp-cov coverage.map *.gcda *.gcno *.gcov
