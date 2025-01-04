.PHONY = all clean install
all:
	@(cd c; make)
	@(cd cpp; make)
clean cl:
	@(cd c; make clean)
	@(cd cpp; make clean)
install:
	@(cd c; make install)
	@(cd cpp; make install)
