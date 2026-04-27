COMPILER = g++
CFLAGS = -O3 -march=native  -Wall -Wextra
DEBUG_FLAGS = -g3 -ggdb -DDEBUG

SRC_DIR = hash_func_choose/hash_tables
SRC = $(SRC_DIR)/hash_func.cpp $(SRC_DIR)/hash_table.cpp hash_func_choose/test_hashes/test_hash.cpp

test_hash: $(SRC)
	$(COMPILER) -O2 $(CFLAGS) $^ -o $@

run_test_hash: 
	./test_hash > hash_func_choose/test_hashes/result.csv

HASH_TABLES = $(OPT_DIR)/hash_tables_func.cpp $(BUCK_DIR)/bucket_func.cpp
MAIN_FILES = $(HASH_TABLES) hash_table_optimize/main_test.cpp 
TEST_HT_FILES = $(HASH_TABLES) hash_table_optimize/test_ht_work.cpp $(OPT_DIR)/hash_table_opt_intr2.cpp
OPT_DIR = hash_table_optimize/hash_tables
BUCK_DIR = hash_table_optimize/hash_tables/bucket_func
OPT_SRC_0 = $(OPT_DIR)/hash_table_no_opt.cpp $(MAIN_FILES)
OPT_SRC_1 = $(OPT_DIR)/hash_table_opt_intr1.cpp $(MAIN_FILES)
OPT_SRC_2 = $(OPT_DIR)/hash_table_opt_intr2.cpp $(MAIN_FILES)
OPT_SRC_3 = $(OPT_DIR)/hash_table_opt_intr3.cpp $(MAIN_FILES)
OPT_EXTRA_3  = $(OPT_DIR)/my_strcmp.o

$(OPT_DIR)/my_strcmp.o: $(OPT_DIR)/my_strcmp.s
	nasm -f elf64  $< -o $@

ht_work_test:
	$(COMPILER) $(CFLAGS) $(TEST_HT_FILES) -o $@ -lbsd

opt_pgo_makeprof0: $(OPT_SRC_0)
	$(COMPILER) $(CFLAGS) -fprofile-generate  -DNDEBUG $^ -o $@ -lbsd

opt_pgo_doprof0: $(OPT_SRC_0)
	$(COMPILER) $(CFLAGS) -fprofile-use -DNDEBUG $^ -o $@ -lbsd

opt_test3: $(OPT_EXTRA_3) $(OPT_SRC_3)
	$(COMPILER) $(CFLAGS) -DNDEBUG $^ -o $@ -lbsd

opt_test%:
	$(COMPILER) $(CFLAGS) -DNDEBUG  $(OPT_SRC_$*) -o $@ -lbsd

opt_test_valgrind3: $(OPT_EXTRA_3) $(OPT_SRC_3)
	$(COMPILER) $(CFLAGS) $(DEBUG_FLAGS) $^ -o $@ -lbsd

opt_test_valgrind%:
	$(COMPILER) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_SRC_$*) $(OPT_EXTRA_$*) -o $@ -lbsd

run_ht_work_test:
	./ht_work_test

run_opt_test%:
	taskset -c 3 ./opt_test$* 100 20 > results.txt && echo "Process ended" >> "control_freq/control.csv" && python3 parse.py opt$* results.txt results.csv

run_pgo_test1:
	taskset -c 3 ./opt_pgo_makeprof0 100 20 > results.txt

run_pgo_test2:
	taskset -c 3 ./opt_pgo_doprof0 100 20 > results.txt  && echo "Process ended" >> "control_freq/control.csv" &&  python3 parse.py pgo results.txt results.csv

run_opt_test_valgrind%:
	taskset -c 3 valgrind --tool=callgrind --cache-sim=yes  ./opt_test_valgrind$* 20 5 > results.txt

view_valgrind:
	kcachegrind callgrind.out.*

clean:
	rm -f opt_test* ht_work_test test_hash opt_pgo_makeprof0 opt_pgo_doprof0 callgrind.out.* *.o *.gcda $(OPT_EXTRA_3) results.txt
