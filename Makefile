all: mb5 mbf

mb5: mandelbrot5_template.o
	gcc mandelbrot5_template.o libmandel.a -lm -o mb5

mbf: mandelbrot_fork.o
	gcc mandelbrot_fork.o libmandel.a -lm -o mbf

mandelbrot5_template.o: mandelbrot5_template.c
	gcc mandelbrot5_template.c -c

mandelbrot_fork.o: mandelbrot_fork.c
	gcc mandelbrot_fork.c -c

clean:
	rm *.o
