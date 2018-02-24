tbox_server: tbox_server.c
	gcc -o tbox_server tbox_server.c \
		-I/home/zsx/download/fiber/tbox/build/tbox \
		-L../../../download/fiber/tbox/build/release/x86_64 -ltbox \
		-ldl -lpthread -lz -lm
clean:
	@(rm -f tbox_server)
