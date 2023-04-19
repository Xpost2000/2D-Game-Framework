gcc -c -O2 prinny_mark_host_old.c -I./..
gcc -shared -O2 prinny_mark_host_old.o -o prinny_mark.dll
