#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
    Constantino Flouras
    April 2nd, 2018
    Matrix Multiplication with pthreads
*/


// Create pointers to int [][].
int ** matrixOne;
int ** matrixTwo;


// Note that MATRIX_ONE_WIDTH has to be equal to MATRIX_TWO_HEIGHT.

#define MATRIX_ONE_WIDTH 10
#define MATRIX_ONE_HEIGHT 5

#define MATRIX_TWO_WIDTH 10
#define MATRIX_TWO_HEIGHT 10
#define MAX_RANDOM_BOUND 20

int fillMatrix(int ** matrix, int width, int height, int limit);
int printMatrix(int ** matrix, int width, int height);
int malloc_2D_array(int *** matrix, int width, int height);
int ** matrix_multiplication(int ** matrixOne, int matrixOneWidth, int matrixOneHeight,
                          int ** matrixTwo, int matrixTwoWidth, int matrixTwoHeight);

int main()
{
    // We'll need to malloc and fill the int[][].

    if ( malloc_2D_array(&matrixOne, MATRIX_ONE_WIDTH, MATRIX_ONE_HEIGHT) != 0 ||
         malloc_2D_array(&matrixTwo, MATRIX_TWO_WIDTH, MATRIX_TWO_HEIGHT) != 0 )
    {
        #ifdef DEBUG
            printf("DEBUG [main()]: Since memory allocation failed, program terminating...\n");
        #endif
        return -1;
    }
    #ifdef DEBUG // This else statement is only necessary if we're debugging.
    else
    {
            printf("DEBUG [main()]: Memory allocation of matrixOne and matrixTwo successful...\n");
    }
    #endif
    //matrixTwo = malloc( sizeof( int[MATRIX_TWO_HEIGHT][MATRIX_TWO_WIDTH] ) );
    
    
    // Now, we'll fill in the arrays with random integers.
    // Note that using srand with the time as the seed is cryptographically
    // insecure, but for our purposes, it is perfectly fine.
    srand( (unsigned int) time(NULL) * 2);
    
    fillMatrix(matrixOne, MATRIX_ONE_WIDTH, MATRIX_ONE_HEIGHT, MAX_RANDOM_BOUND);
    fillMatrix(matrixTwo, MATRIX_TWO_WIDTH, MATRIX_TWO_HEIGHT, MAX_RANDOM_BOUND);
    
    printf("MATRIX #1: \n");
    printMatrix(matrixOne, MATRIX_ONE_WIDTH, MATRIX_ONE_HEIGHT);
    printf("MATRIX #2: \n");
    printMatrix(matrixTwo, MATRIX_TWO_WIDTH, MATRIX_TWO_HEIGHT);
    
    printf("ATTEMPT MATRIX MULTIPLICATION: \n");
    matrix_multiplication(matrixOne, MATRIX_ONE_WIDTH, MATRIX_ONE_HEIGHT, matrixTwo, MATRIX_TWO_WIDTH, MATRIX_TWO_HEIGHT);
    
    return 0;
    
    
    
}

int fillMatrix(int ** matrix, int width, int height, int limit)
{
    #ifdef DEBUG
        printf("DEBUG [fillMatrix()]: Attempting to fill the matrix...");
    #endif
    int numberOfModifiedEntries = 0;
    
    for (int i = 0; i < width * height; i++)
    {
        matrix[i/width][i%width] = rand() % limit;
        numberOfModifiedEntries++;
    }
    
    return numberOfModifiedEntries;
    
}

int printMatrix(int ** matrix, int width, int height)
{
    for (int i = 0; i < height * width; i++)
    {
        printf("%4d%s", matrix[i/width][i%width], (i % width == width - 1) ? "\n" : "");
    }
    
    return 0;
}

int malloc_2D_array(int *** matrix, int width, int height)
{
    // Since matrix is a pointer to a pointer, we'll need to first
    // malloc the space for the initial pointers, and then malloc a whole
    // bunch of pointers within those pointers.
    
    #ifdef DEBUG
        printf("DEBUG [malloc_2D_array()]: Received pointer %p\n", matrix);
    #endif
    
    *matrix = (int **)   malloc( sizeof( int * ) * (height) );
    *matrix[0] = (int *) malloc( sizeof( int   ) * (width * height) );
    
    #ifdef DEBUG
        printf("DEBUG [malloc_2d_array()]: Made it past the malloc() statements...\n");
    #endif
    if (*matrix == NULL || *matrix[0] == NULL)
    {
        // Typical error-checking to ensure that the malloc() was successful
        // If it wasn't, then we'll return a value.
        #ifdef DEBUG
            printf("DEBUG [malloc_2D_array()]: Memory allocation FAILED!\n");
        #endif
        return -1;
    }
    
    // Then, we'll make sure each succeeding pointer goes to the right place
    for (int arrPoint = 1; arrPoint < height; arrPoint++)
    {
        // printf("DEBUG [malloc_2D_array()]: Attempt to allocate the following address: %p\n", (*matrix)[arrPoint]);
        // printf("DEBUG [malloc_2D_array()]:                 to this following address: %p\n", (*matrix) + (width) * (arrPoint));
        
        (*matrix)[arrPoint] =  *(*matrix) + (width) * (arrPoint);
    }
    
    // If debug statements are enabled, this method will print out whether the allocation was successful or not.
    #ifdef DEBUG
        printf("DEBUG [malloc_2D_array()]: Memory allocation was successful.\n");
    #endif
    
    // Return success code 0;
    return 0;
}

int ** matrix_multiplication(int ** matrixOne, int matrixOneWidth, int matrixOneHeight,
                          int ** matrixTwo, int matrixTwoWidth, int matrixTwoHeight)
{
    // First, we'll need to check that we can even do matrix_multiplication on these two particular
    // matrixes. In order to do so, we'll need to ensure that the widths are equal.
    
    // numberOfRows == matrixHeight
    // numberOfColumns == matrixWidth
    
    if (matrixOneWidth != matrixTwoHeight /* Rows in matrixOne must equal columns in matrixTwo */)
    {
        // This cannot be completed
        printf("ERROR [matrix_multiplication()]: The number of rows in matrix one must be equal to number of columns in matrix two!");
    }
    
    
    // Now, we're going to allocate the memory for the result matrix.
    // The resulting matrix will have the same number of rows as matrixOne (aka matrixOneHeight)
    // and the same number of columns (aka matrixTwoWidth).
    
    int ** resultMatrix;
    int resultWidth = matrixTwoWidth;
    int resultHeight = matrixOneHeight;
    
    malloc_2D_array(&resultMatrix, resultWidth , resultHeight);
    
    
    for (int resultRow = 0; resultRow < resultHeight; resultRow++)
    {
        for (int resultColumn = 0; resultColumn < resultWidth; resultColumn++)
        {
            resultMatrix[resultRow][resultColumn] = 0;
            for (int elementNo = 0; (elementNo < matrixOneWidth) && (elementNo < matrixTwoHeight); elementNo++)
            {
                resultMatrix[resultRow][resultColumn] += matrixOne[/*height*/ resultRow][/*width*/ elementNo] * matrixTwo[elementNo][resultColumn];
            }
            
            #ifdef DEBUG
                printf("DEBUG [resultMatrix()]: The result was %d.\n", resultMatrix[resultRow][resultColumn]);
            #endif
        }
    }
    
    
    
    
    
    
    
}


// Stub function. Will do something with it later.
int matrix_mult(int ** matrixOne, int ** matrixTwo, int matrixOneRow, int matrixTwoCol)
{
    return 0;
}