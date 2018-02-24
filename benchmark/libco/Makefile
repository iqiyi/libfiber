libco_server: libco_server.cpp
	g++ -o libco_server libco_server.cpp ../patch.cpp \
		 -I../../../download/fiber/libco/ \
		 -L../../../download/fiber/libco/lib/ -lcolib -ldl -lpthread
clean:
	@(rm -f libco_server)
