#ifndef MANDEL_H
#define MANDEL_H


#define WIDTH 1000	
#define HEIGHT 1000	

typedef struct {
	double xMin;  // minimum real value
	double xMax;  // maximum real value
	double yMin;  // minimum imaginary value
	double yMax;  // maximum imaginary value
	double step;  // step size in real/imaginary
	int width;
	int height;
	int *histogram;  // array for storing histogram values for colouring, dim: maxIter
	int *iterations;  // array for storing number of iterations, dim: WIDTH * HEIGHT
	double *pixels; // array for storing colour value of pixel, dim: WIDTH * HEIGHT
	double complex *carray; // array for storing complex numbers c, dim: WIDTH * HEIGHT
	int maxIter;  // maximum iterations before confident point is in mandelbrot set
} Parameters;


/* function prototypes */
void initialise_lib(Parameters *);
void mandelCompute_lib(Parameters *);
void writeToFile_lib(Parameters);
void histogramColouring_lib(Parameters *p);
void freeMemory_lib(Parameters p);
void parrmandelCompute(Parameters *p);


#endif
