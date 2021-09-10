all: mb5 mbfs mbfp mbp

mb5: mandelbrot5_template.o
	gcc mandelbrot5_template.o libmandel.a -lm -o mb5

mbfs: mandelbrot_forks.o
	gcc mandelbrot_forks.o libmandel.a -lm -o mbfs

mbfp: mandelbrot_forkp.o
	gcc mandelbrot_forkp.o libmandel.a -lm -o mbfp

mbp: mandelbrot_pthread.o
	gcc mandelbrot_pthread.o libmandel.a -lpthread -lm -o mbp

mbomp: mandelbrot_omp.o
	gcc mandelbrot_omp.o libmandel.a -lm -o mbomp
	
mandelbrot5_template.o: mandelbrot5_template.c
	gcc mandelbrot5_template.c -c


clean:
	rm *.o
	rm mb5
	rm mbfp
	rm mbfs
	rm mbp
	rm mandel.dat

