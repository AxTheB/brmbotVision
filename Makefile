%.o: %.c
	gcc -std=c99 -ggdb `pkg-config --cflags opencv` $< -o $@ -c `pkg-config --libs opencv`

avi_view: avi_view.o o2c3conv.o imgutil.o
	gcc -std=c99 -ggdb `pkg-config --cflags opencv` $^ -o $@ `pkg-config --libs opencv`
