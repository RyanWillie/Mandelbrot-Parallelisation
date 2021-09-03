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
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#define N 4

enum{CHILD, PARENT};

/* function prototypes */
void initialise(Parameters *);
void mandelCompute(Parameters *);
void writeToFile(Parameters);
void histogramColouring(Parameters *p);
void freeMemory(Parameters p);
void parrmandelCompute(Parameters *p);
void chread(int fd, char *buf, int count, int chunksize);
void chwrite(int fd, char *buf, int count, int chunksize);


typedef struct{
	int start, end;
} Index;

/* main program – execution begins here */
int main(int argc, char *argv[])
{
	int maxIter;
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
	p.height = HEIGHT;
	p.width = WIDTH;

	printf("xMin = %lf\nxMax = %lf\nyMin = %lf\nyMax = %lf\nMaximum iterations = %i\n", p.xMin, p.xMax, p.yMin, p.yMax, p.maxIter);
	
	initialise(&p);
	parrmandelCompute(&p);
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
	free(p.pixels);
	free(p.carray);
	free(p.iterations);
	free(p.histogram);
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
	//Get the total number of numiterations per pixel
	/*
	printf("	-> Using custom histogramColouring <-\n");
	int total, i, j ,k, iterations;
	for(i = 0; i < p->maxIter; i++){
		total += p->iterations[i];
	}
	printf("Total :%d\n", total);

	for (i = 0; i < p->width; i++) {
		for (j = 0; j < p->height; j++) {
			iterations = p->iterations[i * p->width + j];
			p->histogram[i*p->width + j] += (iterations / total) ;
			printf("Completed: %d %d\n", i, j);
		}
	}
	*/

}

// test each point in the complex plane to see if it is in the set or not
void mandelCompute(Parameters *p)
{
	//mandelCompute_lib(p);
	printf("	-> Using custom mandelCompute <-\n");
	double complex c, z;
	int i,j ,k;

	for(i=0; i < p->height; i++){
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

void parrmandelCompute(Parameters *p){
	int sv[2];
	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) != 0){
		perror("Socket error\n");
	}
	Index idx;

	if(fork() == 0){
		//Child Process
		read(sv[CHILD], &idx, sizeof(Index));
		printf("Startin at %d\n", idx.start);
		p->carray = &(p->carray[idx.start * p->width]);
		p->height /= 2;
		mandelCompute(p);
		p->height *= 2;
		chwrite(sv[CHILD], (char *)p->iterations, p->width * p->height * sizeof(double),1024);
		exit(EXIT_SUCCESS);	
	}else{
		//I'm the parent
		idx.start = p->height/2;
		idx.end = p->width-1;
		write(sv[PARENT], &idx, sizeof(Index));
		p->height /= 2;
		printf("First element: %d\n", p->iterations[0]);
		mandelCompute(p);
		
		//Read in the other half
		p->height *= 2;
		chread(sv[PARENT], (char *)&(p->iterations[idx.start*p->width]), p->width * p->height * sizeof(double), 1024);
		
		wait(NULL);
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

void chwrite(int fd, char *buf, int count, int chunksize)
{
	int numChunks = (int)(count / chunksize);
	int remsize = count - numChunks * chunksize; // number of bytes remaining
	int i, j;
		
	for (i = 0, j = 0; i < numChunks; j += chunksize, i++) {
		//printf("Writing chunk %i out of %i\n", i, numChunks);
		write(fd, &(buf[j]), chunksize);
	}
	
	// write remaining bytes
	write(fd, &(buf[j]), remsize);	
}

void chread(int fd, char *buf, int count, int chunksize)
{
	int numChunks = (int)(count / chunksize);
	int remsize = count - numChunks * chunksize; // number of bytes remaining
	int i, j;
		
	for (i = 0, j = 0; i < numChunks; j += chunksize, i++) {
		//printf("Reading chunk %i out of %i\n", i, numChunks);
		read(fd, &(buf[j]), chunksize);
	}
	
	// write remaining bytes
	read(fd, &(buf[j]), remsize);	
}






