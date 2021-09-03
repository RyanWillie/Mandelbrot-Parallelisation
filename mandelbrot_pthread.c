// Mandelbrot 5
// Written by Stephen So
// This is the fifth version of mandelbrot, rewritten from scratch, to work with gnuplot
// Added carray
// Histogram colour algorithm
// Single-threaded version

// Example coordinates: mandelbrot5 10000 -0.668 0.32 0.02

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include "mandel.h"
#include <pthread.h>

/* function prototypes */
void initialise(Parameters *);
void mandelCompute(Parameters *);
void writeToFile(Parameters);
void histogramColouring(Parameters *p);
void freeMemory(Parameters p);

void pmandelCompute(Parameters *p, int numThreads);
void *dowork(void *argv);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
	int start, end;
	struct Parameters *p; 
} Range;



/* main program – execution begins here */
int main(int argc, char *argv[])
{
	int maxIter, numThreads;
	double xc, yc, size;
	Parameters p;
	
	if (argc < 2) {
		printf("Usage: mandelbrot maxIter [x y size]\n\nUsing default values\n");
		maxIter = 5000;
		p.xMin = p.yMin = -2;
		p.xMax = p.yMax = 2;
	}
	else if (argc == 2) {
		sscanf(argv[1], "%i", &maxIter);
		p.xMin = p.yMin = -2;
		p.xMax = p.yMax = 2;
	}
	else if (argc == 5) {
		sscanf(argv[1], "%i", &maxIter);
		sscanf(argv[2], "%lf", &xc);
		sscanf(argv[3], "%lf", &yc);
		sscanf(argv[4], "%lf", &size);
		
		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
	}

	p.maxIter = maxIter;

	//Number of threads
	numThreads = 2;

	printf("xMin = %lf\nxMax = %lf\nyMin = %lf\nyMax = %lf\nMaximum iterations = %i\n", p.xMin, p.xMax, p.yMin, p.yMax, p.maxIter);
	
	initialise(&p);
	pmandelCompute(&p, numThreads);
	//Compute(&p, numThreads);
	histogramColouring(&p);
	writeToFile(p);
	freeMemory(p);
	
	return (0);
}

// free all dynamic memory in Parameters structure
void freeMemory(Parameters p)
{
	//freeMemory_lib(p);
	free(p.carray);
	free(p.iterations);
	free(p.histogram);
	free(p.pixels);
}

// write coordinates and values to file for gnuplot
void writeToFile(Parameters p)
{
	//writeToFile_lib(p);
	FILE *fp;
	int i, j;
	double complex c;
	
	if((fp = fopen("mandel.dat", "w")) == NULL){
		perror("Cannot open mandel.dat file");
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < HEIGHT; i++){
		for(j = 0; j < WIDTH; j++){
			c = p.carray[i * WIDTH + j];
			fprintf(fp, "%.12lf %.12lf %.12lf\n", creal(c), cimag(c), p.pixels[i * WIDTH + j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

// compute the colour for each pixel using histogram algorithm
void histogramColouring(Parameters *p)
{
	histogramColouring_lib(p);
}

// test each point in the complex plane to see if it is in the set or not
void mandelCompute(Parameters *p)
{
	//mandelCompute_lib(p);
	int i, j, k;
	double complex c, z;

	for(i = 0; i < p->height; i++){
		for(j = 0; j < p->width; j++){
			c = p->carray[i * p->width + j];
			z = 0 + 0 * I;
			for(k = 0; k < p->maxIter; k++){
				z = z * z + c;
				if(cabs(z) > 2.0){
					break;
				}
			}
			if(k >= p->maxIter){
				p->iterations[i * p->width + j] = p->maxIter - 1; //In Mandelbrot set
			}
			else{
				p->iterations[i * p->width + j] = k; //Not in Mandelbrot set
			}
		}
	}
}

void pmandelCompute(Parameters *p, int numThreads)
{
	pthread_t thr[numThreads];
	Parameters pthr[numThreads];
	Range range[numThreads];
	int i;

	pthr[0] = *p;
	pthr[1] = *p;

	//setting range start and end points
	//change this to pthr[i].height = HEIGHT / numThreads -1
	//then need mandelCompute for loop <= instead of <
	range[0].start = 0;
	range[0].end = HEIGHT / 2 - 1;
	range[1].start = HEIGHT / 2;
	range[1].end = HEIGHT;

	//initializing size for carrays
	pthr[0].carray = &(pthr[0].carray[range[0].start * pthr[0].width]);
	pthr[1].carray = &(pthr[1].carray[range[1].start * pthr[1].width]);

	pthr[0].height = range[0].end;
	pthr[1].height = range[1].end;

	pthread_create(&thr[0], NULL, dowork, (void *)&pthr[0]);
	pthread_create(&thr[1], NULL, dowork, (void *)&pthr[1]);

	pthread_join(thr[0], NULL);
	pthread_join(thr[1], NULL);
}

void *dowork(void *arg)
{
	Parameters *p = (Parameters *)arg;
	mandelCompute(p);
	return(NULL);
}

// initialise the Parameters structure and dynamically allocate required arrays
void initialise(Parameters *p)
{
	int i, j;
	double x, y;

	p->step = (p->yMax - p->yMin) / WIDTH;
	
	if ((p->pixels = malloc(WIDTH * HEIGHT * sizeof(double))) == NULL) {
		perror("Cannot allocate memory (pixels)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->carray = malloc(WIDTH * HEIGHT * sizeof(double complex))) == NULL) {
		perror("Cannot allocate memory (pixels)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->iterations = malloc(WIDTH * HEIGHT * sizeof(int))) == NULL) {
		perror("Cannot allocate memory (iterations)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->histogram = malloc(p->maxIter * sizeof(int))) == NULL) {
		perror("Cannot allocate memory (histogram)");
		exit(EXIT_FAILURE);
	}
	
	// initialise array with zeros
	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			p->pixels[i * WIDTH + j] = 0.0;
		}
	}
	
	// initialise carray with real/imaginary values for c
	y = p->yMax;
	for (i = 0; i < HEIGHT; i++) {
		x = p->xMin;
		for (j = 0; j < WIDTH; j++) {
			p->carray[i * WIDTH + j] = x + y * I;
			x += p->step;
		}
		y -= p->step;
	}
	
	for (i = 0; i < p->maxIter; i++) {
		p->histogram[i] = 0;
	}
}