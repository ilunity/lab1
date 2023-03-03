#include <iostream>
#include <mpi.h>
#include <ctime>

const int TAG = 8;

int get_rand(int min, int max) {
    srand(time(0) + rand());

    int num = min + rand() % (max - min + 1);

    return num;
}

void randomize_matrix(int *matrix, int size) {
    for (int i = 0; i < size; ++i) {
        matrix[i] = get_rand(0, 20);
    }
}

int mult_arrays(const int *arr1, const int *arr2, int size) {
    int result = 0;

    for (int i = 0; i < size; ++i) {
        result += arr1[i] * arr2[i];
    }

    return result;
}

void third_program(int rank, int size) {
    const int MATRIX_ROW_NUMBER = 5;
    const int MATRIX_COLUMN_NUMBER = 5;

    const int matrix_size = MATRIX_ROW_NUMBER * MATRIX_COLUMN_NUMBER;
    int *matrix = new int[matrix_size],
            *vector = new int[MATRIX_COLUMN_NUMBER],
            *matrix_row = new int[MATRIX_COLUMN_NUMBER];

    if (rank == 0) {
        randomize_matrix(matrix, matrix_size);
        randomize_matrix(vector, MATRIX_COLUMN_NUMBER);
    }

    // send vector and matrix to other processes
    MPI_Bcast(vector, MATRIX_ROW_NUMBER, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(matrix, MATRIX_COLUMN_NUMBER, MPI_INT, matrix_row, MATRIX_COLUMN_NUMBER, MPI_INT, 0,
                MPI_COMM_WORLD);


    // calculate rows of the resulting matrix
    int row_mult = mult_arrays(matrix_row, vector, MATRIX_COLUMN_NUMBER);
    int *result_matrix = new int[MATRIX_ROW_NUMBER];

    // send pieces of resulting matrix to zero process
    MPI_Gather(&row_mult, 1, MPI_INT, result_matrix, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // print resulting matrix and compare with sequent calculation
    if (rank == 0) {
        printf("Parallel calculations:\n");
        for (int i = 0; i < MATRIX_ROW_NUMBER; ++i) {
            printf("%d ", result_matrix[i]);
        }
        printf("\n");

        int *check_matrix = new int[MATRIX_ROW_NUMBER * MATRIX_COLUMN_NUMBER];
        for (int i = 0; i < MATRIX_ROW_NUMBER; ++i) {
            int row_mult = 0;
            for (int j = 0; j < MATRIX_COLUMN_NUMBER; ++j) {
                row_mult += matrix[i * MATRIX_ROW_NUMBER + j] * vector[j];
            }
            check_matrix[i] = row_mult;
        }

        printf("Sequent calculations:\n");
        for (int i = 0; i < MATRIX_ROW_NUMBER; ++i) {
            printf("%d ", check_matrix[i]);
        }
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    third_program(rank, size);

    MPI_Finalize();
}
