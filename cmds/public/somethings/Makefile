all:
	-mkdir ../man ../man/man1 ../man/man3 ../bin
	./doall; ./doall all; ./doall install
clean:
	./doall clean
clobber:	clean
	./doall clobber
	rm -rf ../man ../bin/* ../*.out
