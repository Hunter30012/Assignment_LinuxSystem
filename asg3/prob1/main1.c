#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX 100

void matrix_multiply(int rowsA, int colsA, int colsB, int A[MAX][MAX], int B[MAX][MAX], int C[MAX][MAX]) 
{
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            C[i][j] = 0;
            for (int k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    int rowsA, colsA, rowsB, colsB;
    int A[MAX][MAX], B[MAX][MAX], C[MAX][MAX];

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

    struct timeval start, end;

    gettimeofday(&start, NULL);
    matrix_multiply(rowsA, colsA, colsB, A, B, C);
    gettimeofday(&end, NULL);
    
    long total_time = end.tv_usec - start.tv_usec;

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
