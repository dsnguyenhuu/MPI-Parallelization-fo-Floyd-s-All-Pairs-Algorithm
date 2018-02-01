/*Dong-Son Nguyen-Huu - 40014054
COMP 428
November 29, 2017

Assignment 3 - Floyd All-Pairs Shortest Path Algorithm - Row and Column-wise
one-to-all broadcasts

*/

#include "mpi.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//TaskID that will identify the master task
#define MASTER 0

//header function that will return the smaller of 2 number values 
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

//Calculates the current position to be used in comparisons for the FW algorithm
void calculatePosition(int i, int n, int* position) {
	position[0] = i % n;
	position[1] = i / n;
}

//Function that will return the index
int getIndex(int x, int y, int nodeNum) {
	return x + (y * nodeNum);
}
//Add 2 numbers together, if any of them are 'inf' then return 'inf'
int intAdd(int a, int b) {
	if (a == INT_MAX || b == INT_MAX) {
		return INT_MAX;
	}
	else {
		return a + b;
	}
}

int main(int argc, char *argv[]) {
	
	//Int values that will be used by the MPI protocol
	int taskID, numTasks, numNodes;

	//Mark the starting point, uses MPI
	double begin = MPI_Wtime();

	//Set up MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskID);

	//Create a random seed using "time"
	srand(time(NULL));

	//Initialize integer values
	int sqrtNumTasks = sqrt(numTasks);
	int numGraphVals = 0;
	int* graphVal;

	//Following statement will only be run by the Master task
	if (taskID == MASTER) {
		//Integer value that will 
		int num;

		//Open a file that contains the weighted graph that will be computed
		FILE *file;
		file = fopen("input.txt", "r");
				
		// Count number of numbers in the weighted graph from the input file in order to determine array size
		// Continues as long as the file is not at the end yet 
		while (!feof(file)) {
			int isInt = fscanf(file, "%d", &num);
			//continues if it detects a non-int element
			if (isInt != 1) { 
				char ch;
				fscanf(file, "%c", &ch);
			}
			
			//iterate the number of values in the graph
			numGraphVals++;
		}

		// Square root of the number of integers in a graph will corespond to teh number of nodes
		numNodes = sqrt(numGraphVals);

		// Array of the size of the number of integers in the graph will be created
		// Memory needs to be allocated totalling the number of elements  * the size of an int
		graphVal = malloc(numGraphVals * sizeof(int));

		// Move the pointer back to the beginning of the file so that it can be used again
		rewind(file);

		int num2 = 0;

		//File will be scanned again to put values in the graph
		while (!feof(file)) {
			int isInt = fscanf(file, "%d", &graphVal[num2]);
			//verifies that the value read is an integer
			if (isInt != 1) { 
				//populate non-integer cells with an infinity
				graphVal[num2] = INT_MAX; 
				char ch;
				fscanf(file, "%c", &ch);
			}
			//iterate
			num2++;
		}
		// Close file to avoid memory leaks
		fclose(file);
	}

	// Use MPI to broadcase the number of nodes and vallues per graph to all tasks
	MPI_Bcast(&numGraphVals, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&numNodes, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	//Sequence will only continue if the calling task is not the master
	if (taskID != MASTER) {
		//allocate enough space for the array that holds the values from the weighted graph
		graphVal = malloc(numGraphVals * sizeof(int));
	}

	// Use MPI to broadcast the array that holds the graph values to all other tasks
	MPI_Bcast(graphVal, numGraphVals, MPI_INT, MASTER, MPI_COMM_WORLD);

	// Generate the number of rows and columns that will determine the buffer size
	int rowColNum = numNodes / sqrtNumTasks;
	int startIndex[sqrtNumTasks];

	startIndex[0] = 0;

	//Counter for for loop, loops through the available tasks
	for (num3 = 1; num3 < sqrtNumTasks; num3++)
		//change the start index based on our buffer size
		startIndex[num3] = startIndex[num3 - 1] + rowColNum;

	// Find the corresponding k for a processor's dimension
	int dimension[numNodes];
	int currentDimension = 0;

	int num4;

	// Iterate through the number of nodes
	for (num4 = 0; num4 < numNodes; num4++) {
		//Increase the size of the current dimension counter if a match is found 
		if (startIndex[currentDimension] + rowColNum == num4) {
			currentDimension++;
		}
		//Populate the dimension array with the value of the current dimension  
		dimension[num4] = currentDimension;
	}

	int position[2];
	calculatePosition(taskID, sqrtNumTasks, position);

	// Place the processe into their respective comm worlds (for both row & columns)
	// Use MPI to split the rows and columns 
	MPI_Comm rowComms, colComms;
	MPI_Comm_split(MPI_COMM_WORLD, position[1], position[0], &rowComms);
	MPI_Comm_split(MPI_COMM_WORLD, position[0], position[1], &colComms);

	//Int values that will correspond to the matrix level, along with the current x and y values
	int k, xVal, yVal;

	//Iterate through all the matrix levels
	for (k = 0; k < numNodes; k++) {

		// Store the dimension that contains this matrix level k
		int kFound = dimension[k];

		// Allocate the buffers accordingly
		int* rowBuffer = malloc(rowColNum * sizeof(int));
		int* colBuffer = malloc(rowColNum * sizeof(int));

		// If a match is found with the y coordinate, then a k needs to be rebroadcasted through MPI
		if (position[1] == kFound) {
			int num5;
			for (num5 = 0; num5 < rowColNum; num5++)	{
				int index = getIndex(startIndex[position[0]] + num5, k, numNodes);
				rowBuffer[num5] = graphVal[index];
			}
		}

		// MPI Column broadcast 
		MPI_Bcast(rowBuffer, rowColNum, MPI_INT, kFound, colComms);

		//If a match is found with the x coordinate, then a k needs to be rebroadcasted through MPI
		if (position[0] == kFound) {
			int num6;
			for (num6 = 0; num6 < rowColNum; num6++) {
				int index = getIndex(k, startIndex[position[1]] + num6, numNodes);
				colBuffer[num6] = graphVal[index];
			}
		}
		
		//MPI Row broadcast
		MPI_Bcast(colBuffer, rowColNum, MPI_INT, kFound, rowComms);

		// Perform Floyd's All-Pairs Shortest Path comparison
		for (xVal = 0; xVal < rowColNum; xVal++) {
			for (yVal = 0; yVal < rowColNum; yVal++) {
				int x = xVal + startIndex[position[0]];
				int y = yVal + startIndex[position[1]];
				//skips the diagonal to avoid the values comparing with themselves
				if (x == k || y == k || x == y) {
					continue;
				}
				//Determine the smallest value from both that are getting compared
				graphVal[getIndex(x, y, numNodes)] = min(graphVal[getIndex(x, y, numNodes)], intAdd(rowBuffer[xVal], colBuffer[yVal]));
			}
		}
		// Deallocates memory
		free(rowBuffer);
		free(colBuffer);
	}

	//Non-Master task function
	if (taskID != MASTER) {
		//Create a buffer with size of the rows/cols
		int sendBufferSize = rowColNum * rowColNum;
		int* sendBuffer = malloc(sendBufferSize * sizeof(int));

		//int values that will correspond to the rows and the columns
		int j, k;
		for (j = 0; j < rowColNum; j++) {
			for (k = 0; k < rowColNum; k++) {
				sendBuffer[j + (k * rowColNum)] = graphVal[getIndex(startIndex[position[0]] + j, startIndex[position[1]] + k, numNodes)];
			}
		}

		// Use MPI to send the buffer 
		MPI_Send(sendBuffer, sendBufferSize, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
		
		//Deallocate the buffer
		free(sendBuffer);
	}
	//Master task function
	else {
		int i = 0;
		for (i = 1; i < numTasks; i++) {

			//Hold the positions of current value being read
			int process_position[2];
			calculatePosition(i, sqrtNumTasks, process_position);

			// Receive the process' elements
			int procArraySz = rowColNum * rowColNum;
			int* procArray = malloc(procArraySz * sizeof(int));
			MPI_Recv(procArray, procArraySz, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			int j, k;

			for (j = 0; j < rowColNum; j++)
				for (k = 0; k < rowColNum; k++)
					graphVal[getIndex(startIndex[process_position[0]] + j,startIndex[process_position[1]] + k,numNodes)] = procArray[j + (k * rowColNum)];

			free(procArray);
		}

		// Store final graph
		FILE *file;
		//Set permission to write
		file = fopen("output.txt", "w");

		int j, k;
		for (j = 0; j < numNodes; j++) {
			for (k = 0; k < numNodes; k++)
				fprintf(file, "%d\t", graphVal[getIndex(k, j, numNodes)]);
			fprintf(file, "\n");
		}
		//Close the file to prevent further unwanted writing
		fclose(file);
	}

	MPI_Finalize();
	if (taskID == MASTER) {
		double stop = MPI_Wtime();
		printf("Run time in ms: %f\n", (stop - begin) * 1000);
	}

	free(graphVal);
	return 0;
}
