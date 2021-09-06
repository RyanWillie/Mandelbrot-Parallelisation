all: mb5 mbfs mbfp

mb5: mandelbrot5_template.o
	gcc mandelbrot5_template.o libmandel.a -lm -o mb5

mbfs: mandelbrot_forks.o
	gcc mandelbrot_forks.o libmandel.a -lm -o mbfs

mbfp: mandelbrot_forkp.o
	gcc mandelbrot_forkp.o libmandel.a -lm -o mbfp

mandelbrot5_template.o: mandelbrot5_template.c
	gcc mandelbrot5_template.c -c

mandelbrot_fork.o: mandelbrot_fork.c
	gcc mandelbrot_fork.c -c

clean:
	rm *.o
	rm mb5
	rm mbfp
	rm mbfs
	rm mandel.dat

