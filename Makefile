
banker: _always_
	g++ -g -Wall -Werror -O1 -o banker scenarios.cc banker.cc -lpthread

.PHONY: _always_
