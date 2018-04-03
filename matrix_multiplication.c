#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
    Constantino Flouras
    April 2nd, 2018
    Matrix Multiplication with pthreads
*/

// We're going to create a typedef struct called matrix.

typedef struct matrix
{
    int ** array;
    int width;
    int height;
} matrix;

// Note that MATRIX_ONE_WIDTH has to be equal to MATRIX_TWO_HEIGHT.
#define MATRIX_ONE_WIDTH 3
#define MATRIX_ONE_HEIGHT 2

#define MATRIX_TWO_WIDTH 4
#define MATRIX_TWO_HEIGHT 3
#define MAX_RANDOM_BOUND 20

#define NUM_THREADS 2;

// Method headers
void init_matrix(matrix * m, int width, int height);
int fillMatrix(matrix m, int limit);
int printMatrix(matrix m);
int malloc_2D_array(int *** matrix, int width, int height);
matrix matrix_multiplication(matrix m, matrix n);


int main()
{
    // Note that using srand with the time as the seed is cryptographically
    // insecure, but for our purposes, it is perfectly fine.
    srand( (unsigned int) time(NULL) * 2);

    // New matrix code
    matrix firstMatrix;
    matrix secondMatrix;

    // Initialize the matrices
    init_matrix(&firstMatrix, MATRIX_ONE_WIDTH, MATRIX_ONE_HEIGHT);
    init_matrix(&secondMatrix, MATRIX_TWO_WIDTH, MATRIX_TWO_HEIGHT);

    // Fill the matrices
    fillMatrix(firstMatrix, MAX_RANDOM_BOUND);
    fillMatrix(secondMatrix, MAX_RANDOM_BOUND);

    // Print out the matrices
    printf("MATRIX #1:\n");
    printMatrix(firstMatrix);

    printf("MATRIX #2:\n");
    printMatrix(secondMatrix);

    // Do the actual matrix multiplication, and store the result in a matrix variable called result.
    matrix result = matrix_multiplication(firstMatrix, secondMatrix);

    // Finally, print out the results.
    printf("RESULT:\n");
    printMatrix(result);

    return 0;



}

/*
    fillMatrix()

    The fillMatrix method takes in a matrix m and a int limit.
    The method will fill the entire matrix with random numbers
    between 1 to limit. It will return the number of modified entries.
*/
int fillMatrix(matrix m, int limit)
{
    #ifdef DEBUG
        printf("DEBUG [fillMatrix()]: Attempting to fill the matrix...\n");
    #endif
    int numberOfModifiedEntries = 0;

    for (int i = 0; i < m.width * m.height; i++)
    {
        (m.array)[ i / m.width][ i % m.width] = 1 + rand() % limit;
        numberOfModifiedEntries++;
    }

    return numberOfModifiedEntries;

}

/*
    printMatrix()

    This method prints out the matrix m, with comma separated values.
*/
int printMatrix(matrix m)
{
    // A simple printf that reports the number of rows and columns that this matrix has.
    printf("This matrix has height %d (rows) and width %d (columns): \n", m.height, m.width);

    for (int i = 0; i < m.height * m.width; i++)
    {
        printf("%5d%s", m.array[i/(m.width)][i%(m.width)],  // Print the element at the location
            (i % (m.width) == (m.width) - 1) ? "\n" : ", ");  // If the last element of the line, add a space.
    }

    // Successfully printed out the matrix.
    return 0;
}

/*
    malloc_2D_array()

    This method will allocate the memory within the 2D array located within
    the (typedef) matrix struct. I purposefully kept this method separate in
    order to keep the code a litte bit cleaner.
*/
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


/*
    The initialize matrix method will take in the matrix pointer itself,
*/
void init_matrix(matrix * m, int width, int height)
{
    (*m).width = width;
    (*m).height = height;
    malloc_2D_array( &( (*m).array), width, height);
}

/*
    matrix_multiplication()

    This method takes two methods, matrix m and matrix n, and multiplies them
    using the typical matrix multiplication algorithm.
*/
matrix matrix_multiplication(matrix m, matrix n)
{
    // First, we'll need to check that we can even do matrix_multiplication on these two particular
    // matrixes. In order to do so, we'll need to ensure that the widths are equal.

    // numberOfRows == matrixHeight
    // numberOfColumns == matrixWidth

    if (m.width != n.height /* Rows in matrixOne must equal columns in matrixTwo */)
    {
        // This cannot be completed
        printf("ERROR [matrix_multiplication()]: The number of rows in matrix one must be equal to number of columns in matrix two!");
    }


    // Now, we're going to allocate the memory for the result matrix.
    // The resulting matrix will have the same number of rows as matrixOne (aka matrixOneHeight)
    // and the same number of columns (aka matrixTwoWidth).

    matrix result;

    init_matrix(&result, n.width, m.height);


    for (int resultRow = 0; resultRow < result.height; resultRow++)
    {
        for (int resultColumn = 0; resultColumn < result.width; resultColumn++)
        {
            result.array[resultRow][resultColumn] = 0;
            for (int elementNo = 0; (elementNo < m.width) && (elementNo < n.height); elementNo++)
            {
                result.array[resultRow][resultColumn] += m.array[/*height*/ resultRow][/*width*/ elementNo] * n.array[elementNo][resultColumn];
            }
        }
    }

    return result;
}

matrix matrix_multiplication_threaded(matrix m, matrix n)
{
    // First, we'll need to check that we can even do matrix_multiplication on these two particular
    // matrixes. In order to do so, we'll need to ensure that the widths are equal.

    // numberOfRows == matrixHeight
    // numberOfColumns == matrixWidth

    if (m.width != n.height /* Rows in matrixOne must equal columns in matrixTwo */)
    {
        // This cannot be completed
        printf("ERROR [matrix_multiplication()]: The number of rows in matrix one must be equal to number of columns in matrix two!");
    }


    // Now, we're going to allocate the memory for the result matrix.
    // The resulting matrix will have the same number of rows as matrixOne (aka matrixOneHeight)
    // and the same number of columns (aka matrixTwoWidth).

    matrix result;

    init_matrix(&result, n.width, m.height);


    // We're going to create threads based on how many rows are in the matrix.
    int incrementor = m.height / NUM_THREADS;

    if (incrementor == 0)
    {
        // There are too many threads to handle this workload. Print out a warning that we'll
        // use a more appropriate number of threads instead.
        #ifdef DEBUG
            printf("DEBUG [matrix_multiplication_threaded()]: Too many threads! Using a smaller number!");
        #endif
    }



    for (int resultRow = 0; resultRow < result.height; resultRow++)
    {
        for (int resultColumn = 0; resultColumn < result.width; resultColumn++)
        {
            result.array[resultRow][resultColumn] = 0;
            for (int elementNo = 0; (elementNo < m.width) && (elementNo < n.height); elementNo++)
            {
                result.array[resultRow][resultColumn] += m.array[/*height*/ resultRow][/*width*/ elementNo] * n.array[elementNo][resultColumn];
            }
        }
    }

    return result;
}
