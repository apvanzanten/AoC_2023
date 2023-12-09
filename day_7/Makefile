BLD_DIR = bld

.PHONY: all run run_tests clean 

# meta-build targets
$(BLD_DIR)/Makefile: CMakeLists.txt 
	@cmake -B $(BLD_DIR)

# build targets
all: $(BLD_DIR)/Makefile
	@cd $(BLD_DIR); $(MAKE) --no-print-directory all

# run targets
run: all
	@$(BLD_DIR)/main

run_tests: all
	@cd $(BLD_DIR); ctest --output-on-failure

# clean targets
clean:
	@cd $(BLD_DIR) && $(MAKE) clean

clean_full:
	@rm -rf $(BLD_DIR)