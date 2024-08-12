#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX 100

typedef struct {
    int row;
    int col;
    int rowsA;
    int colsA;
    int colsB;
    int (*A)[MAX];
    int (*B)[MAX];
    int (*C)[MAX];
} ThreadData;

void *multiply_element(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    int sum = 0;
    for (int k = 0; k < data->colsA; k++) {
        sum += data->A[data->row][k] * data->B[k][data->col];
    }
    data->C[data->row][data->col] = sum;
    pthread_exit(0);
}

int main() {
    int rowsA, colsA, rowsB, colsB;
    int A[MAX][MAX], B[MAX][MAX], C[MAX][MAX];
    int threadIndex;
    pthread_t threads[MAX * MAX];
    ThreadData threadData[MAX * MAX];

    printf("Enter the number of rows and columns for matrix A: ");
    scanf("%d %d", &rowsA, &colsA);

    printf("Enter the elements of matrix A:\n");
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsA; j++) {
            scanf("%d", &A[i][j]);
        }
    }

    printf("Enter the number of rows and columns for matrix B: ");
    scanf("%d %d", &rowsB, &colsB);

    if (colsA != rowsB) {
        printf("Matrix multiplication not possible. Columns of A must equal rows of B.\n");
        return 0;
    }

    printf("Enter the elements of matrix B:\n");
    for (int i = 0; i < rowsB; i++) {
        for (int j = 0; j < colsB; j++) {
            scanf("%d", &B[i][j]);
        }
    }

    threadIndex = 0;
    struct timeval start, end;

    gettimeofday(&start, NULL);
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            threadData[threadIndex].row = i;
            threadData[threadIndex].col = j;
            threadData[threadIndex].rowsA = rowsA;
            threadData[threadIndex].colsA = colsA;
            threadData[threadIndex].colsB = colsB;
            threadData[threadIndex].A = A;
            threadData[threadIndex].B = B;
            threadData[threadIndex].C = C;
            pthread_create(&threads[threadIndex], NULL, multiply_element, &threadData[threadIndex]);
            threadIndex++;
        }
    }
    gettimeofday(&end, NULL);

    long total_time = end.tv_usec - start.tv_usec;

    for (int i = 0; i < threadIndex; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Resultant matrix C:\n");
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
    printf("Time for maxtrix multi in single thread: %ld us\n", total_time);

    return 0;
}
