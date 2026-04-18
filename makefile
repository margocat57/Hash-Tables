COMPILER = g++
CFLAGS = -Wall -Wextra

SRC_DIR = hash_func_choose/hash_tables
SRC = $(SRC_DIR)/hash_func.cpp $(SRC_DIR)/hash_table.cpp hash_func_choose/test_hashes/test_hash.cpp

test_hash: $(SRC)
	$(COMPILER) -O2 $(CFLAGS) $^ -o $@

run_test_hash: 
	./test_hash > hash_func_choose/test_hashes/result.csv
    
OPT_DIR = hash_table_optimize/hash_tables
OPT_SRC = $(OPT_DIR)/hash_table_no_opt.cpp hash_table_optimize/main_test.cpp
OPT_SRC21 = $(OPT_DIR)/hash_table_opt_intr2.cpp hash_table_optimize/main_test.cpp
OPT_SRC22 = $(OPT_DIR)/hash_table_opt_intr22.cpp hash_table_optimize/main_test22.cpp
OPT_SRC3 = $(OPT_DIR)/hash_table_opt_intr3.cpp hash_table_optimize/main_test.cpp


opt_test: $(OPT_SRC)
	$(COMPILER) -O3 -march=native $(CFLAGS) $^ -o $@

opt_test21: $(OPT_SRC21)
	$(COMPILER) -O3 -march=native $(CFLAGS) $^ -o $@

opt_test22: $(OPT_SRC22)
	$(COMPILER) -O3 -march=native $(CFLAGS) $^ -o $@ -lbsd

opt_test3: $(OPT_SRC3)
	$(COMPILER) -O3 -march=native $(CFLAGS) $^ -o $@

opt_test_valgrind: $(OPT_SRC)
	$(COMPILER) -O3 -g3 -ggdb -march=native -DDEBUG -DNDEBUG $(CFLAGS) $^ -o $@

opt_test_valgrind21: $(OPT_SRC21)
	$(COMPILER) -O3 -g3 -ggdb -march=native -DDEBUG -DNDEBUG $(CFLAGS) $^ -o $@

opt_test_valgrind22: $(OPT_SRC22)
	$(COMPILER) -O3 -g3 -ggdb -march=native -DDEBUG -DNDEBUG $(CFLAGS) $^ -o $@ -lbsd

opt_test_valgrind3: $(OPT_SRC3)
	$(COMPILER) -O3 -g3 -ggdb -march=native -DDEBUG -DNDEBUG $(CFLAGS) $^ -o $@

run_opt_test:
	taskset -c 3 ./opt_test 100 20 > results.txt && python3 parse.py no_opt results.txt results.csv

run_opt_test21:
	taskset -c 3 ./opt_test21 100 20 > results.txt && python3 parse.py opt21 results.txt results.csv

run_opt_test22:
	taskset -c 3 ./opt_test22 100 20 > results.txt && python3 parse.py opt22 results.txt results.csv

run_opt_test3:
	taskset -c 3 ./opt_test3 100 20 > results.txt && python3 parse.py opt3 results.txt results.csv

run_opt_test_valgrind:
	taskset -c 3 valgrind --tool=callgrind ./opt_test_valgrind 20 5 > results.txt

run_opt_test_valgrind21:
	taskset -c 3 valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes ./opt_test_valgrind21 20 5 > results.txt

run_opt_test_valgrind22:
	taskset -c 3 valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes ./opt_test_valgrind22 20 5 > results.txt

run_opt_test_valgrind3:
	taskset -c 3 valgrind --tool=callgrind ./opt_test_valgrind3 20 5 > results.txt

view_valgrind:
	kcachegrind callgrind.out.*

clean:
	rm -f test_hash opt_test opt_test2 opt_test21 opt_test22 opt_test3 opt_test_valgrind opt_test_valgrind2 opt_test_valgrind21  opt_test_valgrind22 opt_test_valgrind3 callgrind.out.*  results.txt


# -lbsd