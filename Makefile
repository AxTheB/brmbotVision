avi_view: avi_view.c
	gcc -std=c99 -ggdb `pkg-config --cflags opencv` -o avi_view avi_view.c `pkg-config --libs opencv`
