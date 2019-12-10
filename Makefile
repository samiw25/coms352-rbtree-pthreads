rbtree: rbtree.o
	g++ -Wall -Werror -g -lpthread -lrt rbtree.o -o rbtree

rbtree.o: rbtree.cpp rbtree.h
	g++ -c -lpthread -lrt rbtree.cpp 

clean:
	rm -f rbtree rbtree.o