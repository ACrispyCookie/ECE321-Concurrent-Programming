SHELL := /bin/bash

.PHONY: all build test check clean list

all: build

build:
	./scripts/build_all.sh

test check:
	./scripts/test_all.sh

clean:
	./scripts/clean_all.sh

list:
	@./scripts/list_assignments.sh
