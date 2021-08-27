all: mb5

mb5: mandelbrot5_template.o
	gcc mandelbrot5_template.o libmandel.a -lm -o mb5

mandelbrot5_template.o: mandelbrot5_template.c
	gcc mandelbrot5_template.c -c
clean:
	rm *.o
