#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include "mandel.h"
#include <unistd.h>
#include <omp.h>

/* function prototypes */
void initialise(Parameters *);
void mandelCompute(Parameters *);
void writeToFile(Parameters);
void histogramColouring(Parameters *p);
void freeMemory(Parameters p);

/* main program – execution begins here */
int main(int argc, char *argv[])
{
	int maxIter, numProcess;
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
	else if (argc == 6) {
		sscanf(argv[1], "%i", &maxIter);
		sscanf(argv[2], "%lf", &xc);
		sscanf(argv[3], "%lf", &yc);
		sscanf(argv[4], "%lf", &size);
		sscanf(argv[5], "%i", &numProcess);

		
		size = size / 2;
		p.xMin = xc - size;
		p.yMin = yc - size;
		p.xMax = xc + size;
		p.yMax = yc + size;
	}

	p.maxIter = maxIter;
	p.height = HEIGHT;
	p.width = WIDTH;
	p.numProcess = numProcess;

	printf("xMin = %lf\nxMax = %lf\nyMin = %lf\nyMax = %lf\nMaximum iterations = %i\n", p.xMin, p.xMax, p.yMin, p.yMax, p.maxIter);
	
	initialise(&p);
	mandelCompute(&p);
	histogramColouring(&p);
	writeToFile(p);
	freeMemory(p);
	
	return (0);
}

// free all dynamic memory in Parameters structure
void freeMemory(Parameters p)
{
	printf("	-> Freeing Memory <-\n");
	free(p.pixels);
	free(p.carray);
	free(p.iterations);
	free(p.histogram);
}

// write coordinates and values to file for gnuplot
void writeToFile(Parameters p)
{
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
	printf("	-> Using custom mandelCompute <-\n");
	double complex c, z;
	int i,j ,k;

    #pragma omp parallel for num_threads(p->numProcess) private(i, j, k, c, z) shared(p)
	for(i=0; i < p->height; i++){
		//printf("Hello from process: %d\n", omp_get_thread_num());
		for(j=0; j < p->width; j++){
			z = 0 + 0*I;
			c = p->carray[i * p->width + j];
			for(k=0; k < p->maxIter; k++){
				z = z * z + c;
				if(cabs(z) > 2.0){
					p->iterations[i*p->width + j] = k;
					break;
				}
			}
			if(k >= p->maxIter){
				p->iterations[i * p->width + j] = p->maxIter - 1;
			}
		}			
	}
}


// initialise the Parameters structure and dynamically allocate required arrays
void initialise(Parameters *p)
{
	int i, j;
	double x, y;

	p->step = (p->yMax - p->yMin) / p->width;
	
	if ((p->pixels = malloc(p->width * p->height * sizeof(double))) == NULL) {
		perror("Cannot allocate memory (pixels)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->carray = malloc(p->width * p->height * sizeof(double complex))) == NULL) {
		perror("Cannot allocate memory (pixels)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->iterations = malloc(p->width * p->height * sizeof(int))) == NULL) {
		perror("Cannot allocate memory (iterations)");
		exit(EXIT_FAILURE);
	}
	
	if ((p->histogram = malloc(p->maxIter * sizeof(int))) == NULL) {
		perror("Cannot allocate memory (histogram)");
		exit(EXIT_FAILURE);
	}
	
	// initialise array with zeros
	for (i = 0; i < p->height; i++) {
		for (j = 0; j < p->width; j++) {
			p->pixels[i * p->width + j] = 0.0;
		}
	}
	
	// initialise carray with real/imaginary values for c
	y = p->yMax;
	for (i = 0; i < p->height; i++) {
		x = p->xMin;
		for (j = 0; j < p->width; j++) {
			p->carray[i * p->width + j] = x + y * I;
			x += p->step;
		}
		y -= p->step;
	}
	
	for (i = 0; i < p->maxIter; i++) {
		p->histogram[i] = 0;
	}
}
