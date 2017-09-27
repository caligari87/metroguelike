cc = g++
source = main.cpp
output = ./bin/roguelike

all:
	mkdir -p bin
	${cc} ${source} -lncurses -std=c++17 -Wall -Wextra -o ${output}

run: all
	${output}

clean:
	rm -r ./bin
