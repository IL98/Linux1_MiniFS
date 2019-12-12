FILE_SYSTEM_NAME = MiniFS

run :
	touch $(FILE_SYSTEM_NAME)
	gcc -o initial ./src/initial.c ./src/filesystem.c  ./src/datastructures.c   ./src/commands.c
	./initial
	$(RM) initial
	$(RM) initial.o

	gcc -o server server.c ./src/filesystem.c  ./src/datastructures.c   ./src/commands.c
	gcc -o client client.c