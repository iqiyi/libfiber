.PHONEY = all clean cl rebuild rb

all:
	@(cd fiber; make)
	@(cd server; make)
	@(cd client; make)

clean cl:
	@(cd fiber; make clean)
	@(cd server; make clean)
	@(cd client; make clean)

rebuild rb: cl all
