// Mandelbrot set Generation
// Parrellised using Fork() with Pipes as IPC
// Ryan Williamson s5135470
// Jessy Barber s

// Example coordinates: mandelbrot5 10000 -0.668 0.32 0.02

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include "mandel.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

enum{READ, WRITE};

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
	int start, end, chunksize;
} Index;

/* main program – execution begins here */
int main(int argc, char *argv[])
{
	int maxIter, numProcess;
	double xc, yc, size;
	Parameters p;
	
	if (argc < 2) {
		printf("Usage: mandelbrot maxIter [x y size] numThreads\n\nUsing default values\n");
		maxIter = 5000;
		p.xMin = p.yMin = -2;
		p.xMax = p.yMax = 2;
		numProcess = 4;
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

	int p2c[2], c2p[2], num;
	   if (pipe(c2p) != 0) {
		perror("Pipe error"); 
	}
    if (pipe(p2c) != 0) {
		perror("Pipe error"); 
	}

	Index idx;
	idx.chunksize = p->height / p->numProcess;
	printf("Seperating into chunks of: %d\n", idx.chunksize);

	for(int i = 0; i < p->numProcess; i++){
		if(fork() == 0){
			//Child Process
			read(p2c[READ], &idx, sizeof(Index));
			printf("I'm doing %d\n", idx.start);
			p->carray = &(p->carray[idx.start * p->width]);
			p->height = idx.chunksize;
			mandelCompute(p);
			write(c2p[WRITE], (char *)&idx, sizeof(Index));
			write(c2p[WRITE], (char *)p->iterations, p->width * p->height * sizeof(int));
			printf("Process %d Finished!\n", i);
			exit(EXIT_SUCCESS);
		}else{
			//I'm the parent
			idx.start = i * idx.chunksize; 
			write(p2c[WRITE], &idx, sizeof(Index));
			//Read in the other half
		}
	}



	for(int i=0; i < p->numProcess; i++){
		chread(c2p[READ], (char *)&idx, sizeof(Index), 1024); //Need to know which one i'm reading 
		printf("Using %d\n", idx.start); 
		chread(c2p[READ], (char *)&(p->iterations[idx.start*p->width]), p->width * idx.chunksize * sizeof(int), 1024);
	}
	
	for(int i=0; i < p->numProcess; i++){ //Wait for all processes to finish
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
		//printf("Writing chunk %d out of %d\n", i, numChunks);
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






