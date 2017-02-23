/**
 * NAME: Daniel Burkhart
 * CLASS: CSC 6320
 * DATE: 2-15-17
 */

#define NUMBER_OF_THREADS       15
#define PUZZLE_SIZE             9
#define VALID_TOTAL             9

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * Global variables needed throughout project
 */
int status_map[NUMBER_OF_THREADS];

typedef struct {
    int threadNum;
    int x;
    int y;
} parameters;

int puzzle[PUZZLE_SIZE + 1][PUZZLE_SIZE + 1] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, 5,  3,  4,  6,  7,  8,  9,  1,  2},
    {-1, 6,  7,  2,  1,  9,  5,  3,  4,  8},
    {-1, 1,  9,  8,  3,  4,  2,  5,  6,  7},
    {-1, 8,  5,  9,  7,  6,  1,  4,  2,  3},
    {-1, 4,  2,  6,  8,  5,  3,  7,  9,  1},
    {-1, 7,  1,  3,  9,  2,  4,  8,  5,  6},
    {-1, 9,  6,  1,  5,  3,  7,  2,  8,  4},
    {-1, 2,  8,  7,  4,  1,  9,  6,  3,  5},
    {-1, 3,  4,  5,  2,  8,  6,  1,  7,  9}
};

int board[PUZZLE_SIZE][PUZZLE_SIZE];
pthread_t workers[NUMBER_OF_THREADS];
parameters **params;
pthread_attr_t attributes;
int currThreadNum;
int correct;

/**
 * Function declarations for project.
 */
void readPuzzleFromFile();

void handleThreads();
void createRowThreads();
void createColumnThreads();
void createSubGridThreads();

void *checkRows(void *argument);
void *checkColumns(void *argument);
void *checkSubGrid(void *argument);

void validatePuzzle();
void cleanUp();

/**
 * Main method that acts as driver to program
 * @param argc Count of arguments
 * @param argv Value of arguments
 * @return 0 if successful
 */
int main(int argc, char *argv[]) {
    
    readPuzzleFromFile();
    
    handleThreads();
    
    validatePuzzle();
    
    cleanUp();
    
    return 0;
}

/**
 * Reads the puzzle from a text file.
 */
void readPuzzleFromFile() {
    char filename[100];
    FILE *myFile;
    int i, j;
    
    printf("Please enter your filename:\n");
    scanf("%s", filename);
    myFile = fopen(filename, "r");
    
    if (myFile == NULL) {
        printf("Error Reading File\n");
    }
    for (i = 1; i < PUZZLE_SIZE + 1; i++) {
        for (j = 1; j < PUZZLE_SIZE + 1; j++) {
            fscanf(myFile, "%d,", &puzzle[i][j]);
        }
    }
    
    for (i = 1; i < PUZZLE_SIZE + 1; i++) {
        for (j = 1; j < PUZZLE_SIZE + 1; j++) {
            board[i - 1][j - 1] = puzzle[i][j];
        }
    }
}

/**
 * Function that handles the creation, start, and join of all the threads in this program.
 */
void handleThreads() {
    
    pthread_attr_init(&attributes);
    
    currThreadNum = 0;
    correct = 0;
    
    params = malloc(NUMBER_OF_THREADS * sizeof(parameters *));
    
    int i;
    for (i = 0; i < NUMBER_OF_THREADS; i++) {
        params[i] = malloc(sizeof(parameters));
    }
    
    createRowThreads();
    createColumnThreads();
    createSubGridThreads();
    
    for (i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(workers[i], NULL);
        correct = correct + status_map[i];
    }
}

/**
 * Creates 3 threads for 3 sets of 3 rows.
 */
void createRowThreads() {
    
    int rowCounter;
    for (rowCounter = 0; rowCounter < PUZZLE_SIZE; rowCounter += 3) {
        params[(currThreadNum)]->x = rowCounter;
        params[(currThreadNum)]->y = 0;
        params[(currThreadNum)]->threadNum = (currThreadNum) + 1;
        pthread_create(&workers[(currThreadNum)], &attributes, checkRows, (void *) params[(currThreadNum)]);
        currThreadNum++;
    }
}

/**
 * Creates 3 threads for 3 sets of 3 columns
 */
void createColumnThreads() {
    
    int colCounter;
    for (colCounter = 0; colCounter < PUZZLE_SIZE; colCounter += 3) {
        params[currThreadNum]->x = 0;
        params[currThreadNum]->y = colCounter;
        params[currThreadNum]->threadNum = currThreadNum + 1;
        pthread_create(&workers[currThreadNum], &attributes, checkColumns, (void *) params[currThreadNum]);
        currThreadNum++;
    }
}

/**
 * Creates 9 threads for each of the 3x3 subgrids in the 9x9 puzzle
 */
void createSubGridThreads() {
    
    int row, col;
    int startPoints[3] = {0, 3, 6};
    
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            params[currThreadNum]->x = startPoints[row];
            params[currThreadNum]->y = startPoints[col];
            params[currThreadNum]->threadNum = currThreadNum + 1;
            pthread_create(&workers[currThreadNum], &attributes, checkSubGrid, (void *) params[currThreadNum]);
            currThreadNum++;
        }
    }
}

/**
 * Runner method for row threads. Validates 3 row at a time
 * @param argument
 * The parameter struct that gives us the row we need to start with
 * @return
 * Writes to the status map 1 if valid 0 otherwise
 */
void *checkRows(void *argument) {
    
    parameters *rowData = (parameters *) argument;
    int currRow, rowStart, rowEnd, proofNumber, currCol, verify, currThread, validRowCount;
    
    currThread = rowData->threadNum;
    rowStart = rowData->x;
    rowEnd = rowStart + 3;
    validRowCount = 0;
    
    for (currRow = rowStart; currRow < rowEnd; currRow++) {
        
        verify = 0;
        for (proofNumber = 1; proofNumber <= 9; proofNumber++) {
            for (currCol = 0; currCol < PUZZLE_SIZE; currCol++) {
                if (board[currRow][currCol] == proofNumber) {
                    verify++;
                }
            }
        }
        
        if (verify == VALID_TOTAL) {
            validRowCount++;
        }
    }
    
    if (validRowCount == 3) {
        status_map[currThread - 1] = 1;
    } else if (validRowCount != 3) {
        status_map[currThread - 1] = 0;
    }
    
    pthread_exit(0);
}

/**
 * Runner method for column threads. Validates 3 columns at a time
 * @param argument
 * The parameter struct that gives us the column we need to use
 * @return
 * Writes to the status map 1 if valid 0 otherwise
 */
void *checkColumns(void *argument) {
    
    parameters *columnData = (parameters *) argument;
    int currCol, colStart, colEnd, currThread, proofNumber, currRow, verify, validColCount;
    
    currThread = columnData->threadNum;
    colStart = columnData->y;
    colEnd = colStart + 3;
    validColCount = 0;
    
    for (currCol = colStart; currCol < colEnd; currCol++) {
        
        verify = 0;
        for (proofNumber = 1; proofNumber <= 9; proofNumber++) {
            for (currRow = 0; currRow < PUZZLE_SIZE; currRow++) {
                if (board[currRow][currCol] == proofNumber) {
                    verify++;
                }
            }
        }
        if (verify == VALID_TOTAL) {
            validColCount++;
        }
    }
    
    if (validColCount == 3) {
        status_map[currThread - 1] = 1;
    } else if (validColCount != 3) {
        status_map[currThread - 1] = 0;
    }
    
    pthread_exit(0);
}

/**
 * Runner method for sub-grid threads. Validates a single 3x3 subgrid.
 * @param argument
 * The parameter struct that gives us the row and column we need to use
 * @return
 * Writes to the status map 1 if valid 0 otherwise
 */
void *checkSubGrid(void *argument) {
    
    parameters *subGridData = (parameters *) argument;
    int startRow, startCol, currThread, currRow, currCol, currNum, verify;
    
    startCol = subGridData->y;
    startRow = subGridData->x;
    currThread = subGridData->threadNum;
    
    verify = 0;
    for (currNum = 1; currNum <= 9; currNum++) {
        for (currRow = 0; currRow < 3; currRow++) {
            for (currCol = 0; currCol < 3; currCol++) {
                if (board[currRow + startRow][currCol + startCol] == currNum && (verify + 1) == currNum) {
                    verify++;
                }
            }
        }
    }
    
    if (verify == VALID_TOTAL) {
        status_map[currThread - 1] = 1;
    } else if (verify != VALID_TOTAL) {
        status_map[currThread - 1] = 0;
    }
    
    pthread_exit(0);
}

/**
 * Validates the puzzle by comparing it to the number of threads.
 * If each thread returns true (1) then a valid puzzle would equal the number of threads.
 */
void validatePuzzle() {
    
    if (correct == NUMBER_OF_THREADS) {
        printf("The solution is valid.\n");
    } else {
        printf("The solution is invalid\n");
    }
    
}

/**
 * Deletes allocated memory from heap
 */
void cleanUp() {
    
    int i;
    for (i = 0; i < NUMBER_OF_THREADS; i++) {
        free(params[i]);
    }
    
    free(params);
}
