/*Dong-Son Nguyen-Huu - 40014054
COMP 428
November 29, 2017

Assignment 3 - Floyd All-Pairs Shortest Path Algorithm - Graph Generator

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char *argv[]) {
	//Create a random seed  
	srand(time(NULL));

	//Take 4 parameters from the command line
	int graphSize = strtol(argv[1], NULL, 10);
	int connectionPercent = strtol(argv[2], NULL, 10);
	int maxWeight = strtol(argv[3], NULL, 10);
	int minWeight = strtol(argv[4], NULL, 10);

	//Create a file that will be written into
	FILE *file;
	file = fopen("input.txt", "w");

	//int values that will map the x and y coordinate
	int x, y;

	//iterate through the graph based on supplied size
	for (x = 0; x < graphSize; x++) {
		for (y = 0; y < graphSize; y++) {
			if (x == y) {
				fprintf(file, "%d\t", 0);
				continue;
			}

			//determines the connect percentage
			int connected;

			if (connectionPercent > (rand() % 101)) {
				connected = 1;
			}
			else {
				connected = 0;
			}

			if (connected == 1) {
				fprintf(file, "%d\t", rand() % (maxWeight - minWeight) + minWeight);
			}
			else {
				fprintf(file, "N\t");
			}
		}

		fprintf(file, "\n");
	}

	fclose(file);
}

