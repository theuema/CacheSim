all: cache_controller cache_sim link

cache_controller: cache_controller.c cache_controller.h cache_config.h
	gcc -c -g -std=gnu11 $@.c

cache_sim: cache_sim.cpp cache_sim.h
	g++ -c -g -std=c++11 $@.cpp

link:
	g++ -o cache_sim cache_sim.o cache_controller.o

binclean:
	rm -f cache_sim.o cache_controller.o cache_sim

simclean:
	rm -f *.sim *.pdf

clean:
	rm -f rm -f cache_sim.o cache_controller.o cache_sim *.sim *.pdf
