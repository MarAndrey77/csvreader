CC = gcc
CFLAGS = -I./include
TARGET = csvreader
SOURCE ?= source/data.csv

SRC = src/main.c src/columns.c src/rows.c src/hash_table.c src/csv_reader.c

compile: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(SOURCE)

test: $(TARGET)
	@echo "=== VALID TESTS ==="; \
	for file in tests/valid/*.csv; do \
		out=$${file%.csv}.out; \
		echo "Testing $$file"; \
		./$(TARGET) "$$file" > result.tmp; \
		code=$$?; \
		echo "--- Program output ---"; \
		cat result.tmp; \
		echo "----------------------"; \
		if [ $$code -ne 0 ]; then \
			echo "FAILED: expected exit code 0, got $$code"; \
			exit 1; \
		fi; \
		diff -u "$$out" result.tmp || exit 1; \
		echo "OK"; \
		echo ""; \
	done; \
	echo "=== INVALID TESTS ==="; \
	for file in tests/invalid/*.csv; do \
		out=$${file%.csv}.out; \
		echo "Testing $$file"; \
		./$(TARGET) "$$file" > result.tmp; \
		code=$$?; \
		echo "--- Program output ---"; \
		cat result.tmp; \
		echo "----------------------"; \
		if [ $$code -eq 0 ]; then \
			echo "FAILED: expected non-zero exit code, got 0"; \
			exit 1; \
		fi; \
		diff -u "$$out" result.tmp || exit 1; \
		echo "OK"; \
		echo ""; \
	done; \
	rm -f result.tmp

clean:
	rm -f $(TARGET) result.tmp