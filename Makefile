COMPILER ?= g++

.PHONY: clean test

mkgtk: mkgtk.cpp
	$(COMPILER) -std=c++20 $< -o $@

clean:
	rm -rf mkgtk test

test: mkgtk
	./mkgtk testapp com.heimskr.test Test "Test App"
