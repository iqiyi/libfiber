.PHONEY = all clean cl rebuild rb

all:
	@(cd server; make)
	@(cd client; make)
	@(cd event; make)
	@(cd sleep; make)

clean cl:
	@(cd server; make clean)
	@(cd client; make clean)
	@(cd event; make clean)
	@(cd sleep; make clean)

rebuild rb: cl all
