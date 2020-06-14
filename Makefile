all:
	@(cd c; make)
	@(cd cpp; make)
clean:
	@(cd c; make clean)
	@(cd cpp; make clean)
