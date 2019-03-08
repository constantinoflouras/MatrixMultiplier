#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <strings.h>
#include "debug.h"
/*
    Constantino Flouras
    April 2nd, 2018
    Matrix Multiplication with pthreads
*/

// We're going to create a typedef struct called matrix.
// A matrix contains a double-pointer to an array, as well as an integer width and height
typedef struct matrix
{
    int ** array;
    int width;
    int height;
} matrix;

// Note that MATRIX_ONE_WIDTH has to be equal to MATRIX_TWO_HEIGHT.
#define MATRIX_ONE_WIDTH 500
#define MATRIX_ONE_HEIGHT 1000

#define MATRIX_TWO_WIDTH 1000
#define MATRIX_TWO_HEIGHT 500
#define MAX_RANDOM_BOUND 20

#define NUM_THREADS 16

// Method headers
void init_matrix(matrix * m, int width, int height);
int fillMatrix(matrix m, int limit);
int printMatrix(matrix m);
int malloc_2D_array(int *** matrix, int width, int height);
matrix matrix_multiplication(matrix m, matrix n);
matrix matrix_multiplication_threaded(matrix m, matrix n);
void calculate_thread_boundaries(int * start, int * end, int threadNum, int workAmount);
void * pthreaded_calculate_results(void * arguments);

struct pthreaded_calculate_results_args
{
    int start;
    int end;
    int threadNum;
    matrix * m;
    matrix * n;
    matrix * r;
};

struct setup_values
{
    //  These two must be equal
    int n_matrix_two_height;
    int n_matrix_one_width;

    //  These don't matter
    int n_matrix_one_height;
    int n_matrix_two_width;

    //  Number of threads
    int n_number_of_threads;
};

/*
    This is a simple compare strings method that I created for now.
    In retrospect, I totally could have used strncmp.... or not lol
*/



int handle_arguments(int argc, char * argv[], struct setup_values * setup_values)
{
    // Now, we're going to go ahead and parse the input values. We'll do this using a loop.
    debug_printf("Parsing command line arguments...\n");
    int argNum = 0;

    while (argNum < argc)
    {
        debug_printf("Processing argument number %d...\n", argNum);

        if (!strncmp(argv[argNum], "--m1", 4))
        {
            // The following argument is the matrix size. Now, assuming that the input is proper,
            // we'll go ahead and pull the size from the following argument.
            // TODO: FIX THIS!
            debug_printf("Detected argument --m1\n");

            /*  Logic for processing this argument needs to go here...  */
            argNum++;
        }
        //printf("DEBUG: The test output for argument %d is %d\n", argNum, test);
        argNum++;
    }
}

int main(int argc, char * argv[])
{
    // The following are used to store the starting and stopping times
    struct timeval start, stop;

    struct setup_values setup_values;

    if (!handle_arguments(argc, argv, &setup_values))
    {
        printf("No arguments or invalid arguments were supplied. Here's a helpful guide:\n"
            "./matrix_multiplication --m1 100x100 --m2 200x200 --numThreads 2\n"
            "\n--m1\tsize of matrix 1, width x height."
            "\n--m2\tsize of matrix 2, width x height."
            "\n\nWhen specifying the size of the matrixes\n - the width of matrix 1 must match the height of matrix 2."
            "\n - Separate the width and height with an \"x\", no space between numbers and the \"x\".\n\n");
        return -1;
    };

    debug_printf("DEBUG [main()]: Finished parsing command line arguments.");


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
    // printf("MATRIX #1:\n");
    // printMatrix(firstMatrix);

    // printf("MATRIX #2:\n");
    // printMatrix(secondMatrix);

    // Do the actual matrix multiplication, and store the result in a matrix variable called result.
    gettimeofday(&start, NULL);
    matrix result = matrix_multiplication_threaded(firstMatrix, secondMatrix);
    (void) result;
    gettimeofday(&stop, NULL);

    // Finally, print out the results.
    printf("RESULT:\n");
    printf("Matrix %dx%d X Matrix %d,%d with %d threads took %lu.%lu \n",
        firstMatrix.width, firstMatrix.height,
        secondMatrix.width, secondMatrix.height,
        NUM_THREADS, (stop.tv_sec - start.tv_sec), ((stop.tv_usec - start.tv_usec) / 1000));
    //printMatrix(result);

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
        printf("ERROR [matrix_multiplication()]: The number of rows in matrix one %s",
        "must be equal to number of columns in matrix two!");
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

    // Implement the code for threading.
    // 22 / 4 = Minimum of 5 per thread
    // 22 % 4 = Two threads will have an additional row.
    #ifdef DEBUG
        printf("DEBUG [matrix_multiplication_threaded()]: The number of threads with one extra row: %d\n",
            m.height % NUM_THREADS);
        printf("DEBUG [matrix_multiplication_threaded()]: The minimum number of rows per thread is: %d\n",
            m.height / NUM_THREADS);
    #endif
    if ( (m.height / NUM_THREADS) == 0)
    {
        // There are too many threads to handle this workload. Print out a warning that we'll
        // use a more appropriate number of threads instead.
        #ifdef DEBUG
            printf("DEBUG [matrix_multiplication_threaded()]: Too many threads! Using a smaller number!");
        #endif
    }

    pthread_t matrix_threads[NUM_THREADS];

    for (int thread = 0; thread < NUM_THREADS ; thread++)
    {
        int start;
        int end;

        calculate_thread_boundaries(&start, &end, thread, m.height);

        // printf("THREAD #%d: Range is row %d to %d. \n", thread, start+1, end+1);

        // Build the struct.

        /*
            struct pthreaded_calculate_results_args
            {
                int start;
                int finish;
                matrix * m;
                matrix * n;
                matrix * r;
            };
        */

        struct pthreaded_calculate_results_args * a;

        a = malloc(sizeof (struct pthreaded_calculate_results_args));

        (*a).start = start;
        (*a).end = end;
        (*a).m = &m;
        (*a).n = &n;
        (*a).r = &result;
        (*a).threadNum = thread;

        pthread_create(&(matrix_threads[thread]), NULL, pthreaded_calculate_results, a);
    }

    for (int thread = 0; thread < NUM_THREADS; thread++)
    {
        pthread_join(matrix_threads[thread], NULL);
    }

    return result;
}

void calculate_thread_boundaries(int * start, int * end, int threadNum, int workAmount)
{
    /*
    // The minimum number of rows per thread.
    int min_per_thread = m.height / NUM_THREADS;

    // Number of threads that will have one extra.
    int threads_with_extra = m.height % NUM_THREADS;
    */

    *start = (  (threadNum) < (workAmount%NUM_THREADS) ?  (threadNum) * ((workAmount/NUM_THREADS)+1)     :
                 (workAmount%NUM_THREADS * ((workAmount/NUM_THREADS)+1)) + (   ((threadNum) - workAmount%NUM_THREADS) * (workAmount/NUM_THREADS))   )   ;

    *end = (  (threadNum+1) < (workAmount%NUM_THREADS) ?  (threadNum+1) * ((workAmount/NUM_THREADS)+1)     :
                 (workAmount%NUM_THREADS * ((workAmount/NUM_THREADS)+1)) + (   ((threadNum+1) - workAmount%NUM_THREADS) * (workAmount/NUM_THREADS))   )    - 1;
}


/*
    This function is STRICTLY for pthreads ONLY.
*/
void * pthreaded_calculate_results(void * arguments)
{
    struct pthreaded_calculate_results_args * a = (struct pthreaded_calculate_results_args *) arguments;

    int start = (*a).start;
    int finish = (*a).end;
    int threadNum = (*a).threadNum;


    for (int resultRow = start; resultRow <= finish; resultRow++)
    {
        for (int resultColumn = 0; resultColumn < (*(*a).r).width; resultColumn++)
        {
            (*(*a).r).array[resultRow][resultColumn] = 0;
            for (int elementNo = 0; (elementNo < (*(*a).m).width) && (elementNo < (*(*a).n).height); elementNo++)
            {
                (*(*a).r).array[resultRow][resultColumn] += (*(*a).m).array[/*height*/ resultRow][/*width*/ elementNo] * (*(*a).n).array[elementNo][resultColumn];
            }
        }
    }

    #ifdef DEBUG
        printf("DEBUG [pthreaded_calculate_results()]: THREAD %d has finished calculating rows %d to %d.\n", threadNum, start, finish);
    #endif

    free(a);

    return NULL;
}
