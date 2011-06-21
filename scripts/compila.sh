gcc -Wall -c -O list.c
gcc -Wall -c -O semaph.c
gcc -Wall client.c -o client
gcc -Wall replacement_daemon.c semaph.o list.o -o dm
gcc -Wall shutdown_manager.c list.o -o sd
gcc -Wall memory_manager.c semaph.o list.o -o mm

