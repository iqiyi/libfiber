libgo_server: libgo_server.cpp
	g++ -std=c++11 -o libgo_server libgo_server.cpp ../patch.cpp \
		 -DBOOST_COROUTINES_NO_DEPRECATION_WARNING \
		 -I../../../download/fiber/libgo/libgo/linux \
		 -I../../../download/fiber/libgo \
		 -L../../../download/fiber/libgo/build -llibgo \
		 -Wl,-rpath,../../../download/fiber/libgo/build \
		 -lboost_coroutine -lboost_thread -lboost_system \
		 -ldl -Wl,-rpath,/usr/local/lib -lpthread
clean:
	@(rm -f libgo_server)
