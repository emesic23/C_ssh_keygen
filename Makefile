CC = clang
# CFLAGS = -Iinclude -Wall -Wextra -O3 -g #Version 1
CFLAGS = -Iinclude -fsanitize=address,undefined -g -fno-omit-frame-pointer -lm #Version 2
# CFLAGS = -Iinclude -g -O3 -DNDEBUG -lm #Version 3

bin/keygen: out/rsa.o out/base64.o out/keygen.o out/rsa_private_key.o out/utils.o bin/libbigint.a
	$(CC) $(CFLAGS) $^ -o $@

bin/test: out/test.o out/bigint.o out/random.o out/utils.o
	$(CC) $(CFLAGS) $^ -o $@

test: bin/test

bin/libbigint.a: out/bigint.o out/random.o out/utils.o
	ar -cr $@ $^

out/%.o: src-given/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f bin/*
	rm -f out/*
