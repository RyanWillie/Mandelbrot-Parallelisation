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
#include <unistd.h>
#include <sys/types.h>


/* function prototypes */
void initialise(Parameters *);
void mandelCompute(Parameters *);
void writeToFile(Parameters);
void histogramColouring(Parameters *p);
void freeMemory(Parameters p);
void parrmandelCompute(Parameters *p);
void *doWork(void *argv);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
	int start, chunkSize;
} Index;



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
		p.numProcess = 2;
	}
	else if (argc == 2) {
		sscanf(argv[1], "%i", &maxIter);
		p.xMin = p.yMin = -2;
		p.xMax = p.yMax = 2;
	}
	else if (argc == 6) {
		sscanf(argv[1], "%i", &maxIter);
		sscanf(argv[2], "%lf", &xc);
		sscanf(argv[3], "%lf", &yc);
		sscanf(argv[4], "%lf", &size);
		sscanf(argv[5], "%d", &numThreads);
		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
		p.numProcess = numThreads;
	}

	p.maxIter = maxIter;
	p.height = HEIGHT;
	p.width = WIDTH;
	//Number of threads

	printf("xMin = %lf\nxMax = %lf\nyMin = %lf\nyMax = %lf\nMaximum iterations = %i\n", p.xMin, p.xMax, p.yMin, p.yMax, p.maxIter);
	
	initialise(&p);
	parrmandelCompute(&p);
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
	printf("	-> Freeing Memory <-\n");
	free(p.carray);
	free(p.iterations);
	free(p.histogram);
	free(p.pixels);
}

// write coordinates and values to file for gnuplot
void writeToFile(Parameters p)
{
	//writeToFile_lib(p);
	printf("	-> Using custom writeToFile <-\n");
	FILE *fp;
	double complex c;
	
	//Attempt to open the file
	if((fp = fopen("mandel.dat", "w")) == NULL){
		perror("Cannot open mandel.dat file");
		exit(EXIT_FAILURE);
	}

	for(int i=0; i < p.height; i++){
		for(int j=0; j < p.width; j++){
			c = p.carray[i * p.width + j];
			fprintf(fp, "%.12lf %.12lf %.12lf\n", creal(c), cimag(c), p.pixels[i*p.width + j]);
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

void *doWork(void *arg)
{
	Parameters *p = (Parameters *)arg;
	mandelCompute(p);
	return(NULL);
}

void parrmandelCompute(Parameters *p)
{
	pthread_t thr[p->numProcess];
	Parameters pthr[p->numProcess];
	int chunkSize = p->height / p->numProcess;
	int start;

	for(int i=0; i < p->numProcess; i++){
		start = i * chunkSize;
		printf("Creating Thread %d starting at %d with a chunkSize of %d\n", i, start, p->height);
		pthr[i].carray = &(p->carray[start * p->width]);
		pthr[i].height = chunkSize;
		pthr[i].width = p->width;
		pthr[i].maxIter = p->maxIter;
		pthr[i].iterations = &(p->iterations[start * p->width]);
		pthread_create(&thr[i], NULL, doWork, (void *)&pthr[i]);
	}

	
	for(int i=0; i < p->numProcess; i++){
		pthread_join(thr[i], NULL);
	}
	printf("Threads Joined\n");
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