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

/*
    Typedef: matrix
    Description:
        Contains all of the contents of a dynamically allocated matrix.
        The matrix itself is actually allocated as a 2D array, and thus we'll
        need to use malloc/free commands in its initialization.
*/
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

/*
    Function declarations
*/
void init_matrix(matrix * m, int width, int height);
int fillMatrix(matrix m, int limit);
int printMatrix(matrix m);
int malloc_matrix_arr(int *** matrix, int width, int height);
matrix matrix_multiplication(matrix m, matrix n);
matrix matrix_multiplication_threaded(matrix m, matrix n, int num_threads);
void calculate_thread_boundaries(int * start, int * end, int threadNum, int workAmount, int num_threads);
void * pthreaded_calculate_results(void * arguments);
int convert_matrix_text_representation(char * str, int * n_matrix_width, int * n_matrix_height);

struct pthreaded_calculate_results_args
{
    int start;
    int end;
    int threadNum;
    matrix * m;
    matrix * n;
    matrix * r;
};

/*
    Struct: setup_values
    Description:
        Contains the setup values for this run of the program.
        Ideally, these should be configured through arguments passed,
        but if those are invalid, we can also use default values instead.
        The n_matrix_one_width and the n_matrix_two_height must be equal.
*/
struct setup_values
{
    int n_matrix_one_width;     /*  Matrix #1 arguments */
    int n_matrix_one_height;
    int n_matrix_two_height;    /*  Matrix #2 arguments */
    int n_matrix_two_width;
    int n_number_of_threads;    /*  Number of threads   */
};

/*
    Function: handle_arguments
    Description:
        Given the argument counter and array of arguments, set up the arguments
        for this program run and store them in a setup_values structure of which
        is pointed to by the setup_values pointer.
*/
int handle_arguments(int argc, char * argv[], struct setup_values * setup_values)
{
    debug_printf("Parsing command line arguments...\n");
    
    /*  Iterate through all arguments   */
    int argNum = 0;
    while (argNum < argc)
    {
        /*  Matrix one configuration    */
        if (!strncmp(argv[argNum], "--m1", 4))
        {
            debug_printf("Detected argument --m1\n");
            argNum++;
            if (convert_matrix_text_representation(argv[argNum],
                &(setup_values->n_matrix_one_width), &(setup_values->n_matrix_one_height)))
            {
                /*  Failure    */
                printf("Invalid value specified for --m1\n");
                setup_values->n_matrix_one_width = 0;
                setup_values->n_matrix_one_height = 0;
                return -1;
            }
            debug_printf("Grabbed the values for matrix 1: %d x %d\n", setup_values->n_matrix_one_width, setup_values->n_matrix_one_height);
        }
        else if (!strncmp(argv[argNum], "--m2", 4))
        {
            debug_printf("Detected argument --m2\n");
            argNum++;
            if (convert_matrix_text_representation(argv[argNum],
                &(setup_values->n_matrix_two_width), &(setup_values->n_matrix_two_height)))
            {
                /*  Failure    */
                printf("Invalid value specified for --m2\n");
                setup_values->n_matrix_two_width = 0;
                setup_values->n_matrix_two_height = 0;
                return -1;
            }
            debug_printf("Grabbed the values for matrix 2: %d x %d\n", setup_values->n_matrix_two_width, setup_values->n_matrix_two_height);
        }
        else if (!strncmp(argv[argNum], "--numThreads", 12))
        {
            /*  Number of threads passed */
            debug_printf("Detected argument --numThreads\n");
            argNum++;
            
            int cntr = 0;
            setup_values->n_number_of_threads = 0;
            while (argv[argNum][cntr] != '\0')
            {
                setup_values->n_number_of_threads = (setup_values->n_number_of_threads * 10) + (argv[argNum][cntr] - '0');
                cntr++;
            }
            printf("The number of threads is: %d\n", setup_values->n_number_of_threads);
            
        }
        argNum++;
    }
    
    if (setup_values->n_number_of_threads <= 0)
    {
        debug_printf("Invaid number of threads! Reset to 1.\n");
        setup_values->n_number_of_threads = 1;
    }
    
    return 0;
}

/*
    Function: convert_matrix_text_representation
    Description:
        Takes a null-terminated string input that represents a matrix
        (e.g. 100x100) and converts it to two integers-- width and height.
        Returns 0 upon success, 1 upon failure.
*/
int convert_matrix_text_representation(char * str, int * n_matrix_width, int * n_matrix_height)
{
    debug_printf("Received the following string: %s\n", str);
    
    int cntr = 0;
    int x_break = -1;    /*  Location of the 'x', -1 until found    */
    int str_length = 0;
    *n_matrix_width = 0;
    *n_matrix_height = 0;
    
    /*  Determine the location of the 'x' character, and split the string based on that.*/
    while (str[cntr] != '\0')
    {
        str_length += 1;
        
        if (str[cntr] == 'x')
        {
            x_break = cntr;
        }
        else
        {
            /* Based on whether we found the x_break, build the n_matrix_width
                or the n_matrix_height. */
            if (x_break == -1)
                *n_matrix_width = (*n_matrix_width * 10) + (str[cntr] - '0');
            else
                *n_matrix_height = (*n_matrix_height * 10) + (str[cntr] - '0');
        }
        
        cntr++;
    }
    
    return (x_break == -1);
}


int main(int argc, char * argv[])
{
    // The following are used to store the starting and stopping times
    struct timeval start, stop;

    struct setup_values setup_values;

    /*  Attempt to handle the arguments. If there is an error, explicitly say so.   */
    if (handle_arguments(argc, argv, &setup_values))
    {
        printf("No arguments or invalid arguments were supplied. Here's a helpful guide:\n"
            "./matrix_multiplication --m1 100x100 --m2 200x200 --numThreads 2\n"
            "\n--m1\tsize of matrix 1, width x height."
            "\n--m2\tsize of matrix 2, width x height."
            "\n\nWhen specifying the size of the matrixes\n - the width of matrix 1 must match the height of matrix 2."
            "\n - Separate the width and height with an \"x\", no space between numbers and the \"x\".\n\n");
        return -1;
    };

    debug_printf("DEBUG [main()]: Finished parsing command line arguments.\n");


    // Note that using srand with the time as the seed is cryptographically
    // insecure, but for our purposes, it is perfectly fine.
    srand( (unsigned int) time(NULL) * 2);

    /*  Declare, create, and initialize the matrices... */
    matrix firstMatrix;
    matrix secondMatrix;
    init_matrix(&firstMatrix, setup_values.n_matrix_one_width, setup_values.n_matrix_one_height);
    init_matrix(&secondMatrix, setup_values.n_matrix_two_width, setup_values.n_matrix_two_height);

    /*  Fill the matrices with random numerical data based on a maximum bound...*/
    fillMatrix(firstMatrix, MAX_RANDOM_BOUND);
    fillMatrix(secondMatrix, MAX_RANDOM_BOUND);

    /*  TODO: Add a flag that allows printing out the matrix  */

    // Do the actual matrix multiplication, and store the result in a matrix variable called result.
    gettimeofday(&start, NULL);
    matrix result = matrix_multiplication_threaded(firstMatrix, secondMatrix, setup_values.n_number_of_threads);
    (void) result;
    gettimeofday(&stop, NULL);

    // Finally, print out the results.
    printf("RESULT:\n");
    printf("Matrix %dx%d X Matrix %d,%d with %d threads took %lu.%lu \n",
        firstMatrix.width, firstMatrix.height,
        secondMatrix.width, secondMatrix.height,
        setup_values.n_number_of_threads, (stop.tv_sec - start.tv_sec), ((stop.tv_usec - start.tv_usec) / 1000));
    //printMatrix(result);

    return 0;



}

/*
    Function: fillMatrix()
    Description:
        The fillMatrix method takes in a matrix m and a int limit.
        The method will fill the entire matrix with random numbers
        between 1 to limit. It will return the number of modified entries.
*/
int fillMatrix(matrix m, int limit)
{
    debug_printf("Attempting to fill the matrix...\n");
    
    int numberOfModifiedEntries = 0;

    for (int i = 0; i < m.width * m.height; i++)
    {
        (m.array)[ i / m.width][ i % m.width] = 1 + rand() % limit;
        numberOfModifiedEntries++;
    }

    return numberOfModifiedEntries;

}

/*
    Function: printMatrix()
    Desription:
        Prints out the matrix m, with comma separated values.
*/
int printMatrix(matrix m)
{
    printf("This matrix has height %d (rows) and width %d (columns): \n", m.height, m.width);

    for (int i = 0; i < m.height * m.width; i++)
    {
        printf("%5d%s", m.array[i/(m.width)][i%(m.width)],  // Print the element at the location
            (i % (m.width) == (m.width) - 1) ? "\n" : ", ");  // If the last element of the line, add a space.
    }

    /*  Return success  */
    return 0;
}

/*
    Function:   malloc_matrix_arr()
    Description:
        This method will allocate the memory within the 2D array located within
        the (typedef) matrix struct. I purposefully kept this method separate in
        order to keep the code a litte bit cleaner.
*/
int malloc_matrix_arr(int *** matrix, int width, int height)
{
    /*
        The array contained within the matrix structure is a 2D array.
        In order to change the 2D array pointer, such as in a memory allocation,
        use a pointer to a 2D array pointer.... or a * to a **, which is a ***.
    */
    debug_printf("Received pointer %p\n", matrix);
    
    /*
        Step #1: Allocate memory to store the pointers. These pointers are for
        each "height" with the matrix itself.
        
        array[][]
        0:      space to store pointer to array[]
        1:      space to store pointer to array[]
        2:      space to store pointer to array[]
        ..............
        HEIGHT: space to store pointer to array[]
    */
    *matrix = (int **)   malloc( sizeof( int * ) * (height) );
    
    /*
        Step #2: Allocate the memory for each of those individual int arrays.
        Since we know that these addresses are contiguous, we're going to abuse
        that property by allocating these all in one step.
        There's a chance that this might be an unsafe operation-- but since we
        aren't threaded and we're working within our own virtual address space,
        we should be perfectly fine.
    */
    *matrix[0] = (int *) malloc( sizeof( int   ) * (width * height) );

    debug_printf("Made it past the malloc() statements...\n");
    
    if (*matrix == NULL || *matrix[0] == NULL)
    {
        // Typical error-checking to ensure that the malloc() was successful
        // If it wasn't, then we'll return a value.
        #ifdef DEBUG
            printf("DEBUG [malloc_matrix_arr()]: Memory allocation FAILED!\n");
        #endif
        return -1;
    }

    // Then, we'll make sure each succeeding pointer goes to the right place
    for (int arrPoint = 1; arrPoint < height; arrPoint++)
    {
        // printf("DEBUG [malloc_matrix_arr()]: Attempt to allocate the following address: %p\n", (*matrix)[arrPoint]);
        // printf("DEBUG [malloc_matrix_arr()]:                 to this following address: %p\n", (*matrix) + (width) * (arrPoint));

        (*matrix)[arrPoint] =  *(*matrix) + (width) * (arrPoint);
    }

    // If debug statements are enabled, this method will print out whether the allocation was successful or not.
    #ifdef DEBUG
        printf("DEBUG [malloc_matrix_arr()]: Memory allocation was successful.\n");
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
    malloc_matrix_arr( &( (*m).array), width, height);
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

matrix matrix_multiplication_threaded(matrix m, matrix n, const int num_threads)
{
    debug_printf("The number of threads is: %d\n", num_threads);
    // First, we'll need to check that we can even do matrix_multiplication on these two particular
    // matrixes. In order to do so, we'll need to ensure that the widths are equal.

    // numberOfRows == matrixHeight
    // numberOfColumns == matrixWidth

    if (m.width != n.height /* Rows in matrixOne must equal columns in matrixTwo */)
    {
        // This cannot be completed
        debug_printf(" (ERROR) The number of rows in matrix one must be equal to number of columns in matrix two!");
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
            m.height % num_threads);
        printf("DEBUG [matrix_multiplication_threaded()]: The minimum number of rows per thread is: %d\n",
            m.height / num_threads);
    #endif
    if ( (m.height / num_threads) == 0)
    {
        // There are too many threads to handle this workload. Print out a warning that we'll
        // use a more appropriate number of threads instead.
        #ifdef DEBUG
            printf("DEBUG [matrix_multiplication_threaded()]: Too many threads! Using a smaller number!");
        #endif
    }

    pthread_t matrix_threads[num_threads];

    for (int thread = 0; thread < num_threads ; thread++)
    {
        int start;
        int end;

        calculate_thread_boundaries(&start, &end, thread, m.height, num_threads);

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

    for (int thread = 0; thread < num_threads; thread++)
    {
        pthread_join(matrix_threads[thread], NULL);
    }

    return result;
}

void calculate_thread_boundaries(int * start, int * end, int threadNum, int workAmount, const int num_threads)
{
    /*
    // The minimum number of rows per thread.
    int min_per_thread = m.height / num_threads;

    // Number of threads that will have one extra.
    int threads_with_extra = m.height % num_threads;
    */

    *start = (  (threadNum) < (workAmount%num_threads) ?  (threadNum) * ((workAmount/num_threads)+1)     :
                 (workAmount%num_threads * ((workAmount/num_threads)+1)) + (   ((threadNum) - workAmount%num_threads) * (workAmount/num_threads))   )   ;

    *end = (  (threadNum+1) < (workAmount%num_threads) ?  (threadNum+1) * ((workAmount/num_threads)+1)     :
                 (workAmount%num_threads * ((workAmount/num_threads)+1)) + (   ((threadNum+1) - workAmount%num_threads) * (workAmount/num_threads))   )    - 1;
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
