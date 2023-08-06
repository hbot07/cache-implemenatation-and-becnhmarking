compile:
	g++ col216ass3.cpp -std=c++11 -o cache_simulate

run_all: compile
	for i in {1..8}; do \
		echo "Running trace file $$i"; \
		./cache_simulate 64 1024 2 65536 8 memory_trace_files/trace$$i.txt; \
		echo " "; \
	done

run: compile
	@echo "Running trace file: $(TRACE_FILE)"
	./cache_simulate 64 1024 2 65536 8 $(TRACE_FILE)
