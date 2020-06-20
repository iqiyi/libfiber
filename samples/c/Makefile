.PHONEY = all clean cl rebuild rb

all:
	@(cd server; make)
	@(cd client; make)
	@(cd event; make)
	@(cd sleep; make)
	@(cd poll; make)
	@(cd threads; make)
	@(cd dns; make)

clean cl:
	@(cd server; make clean)
	@(cd client; make clean)
	@(cd event; make clean)
	@(cd sleep; make clean)
	@(cd poll; make clean)
	@(cd threads; make clean)
	@(cd dns; make clean)

rebuild rb: cl all
