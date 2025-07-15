// game_of_life_mpi_omp.cpp
#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h> // para usleep

#define ALIVE 1
#define DEAD  0

using namespace std;

int idx(int i, int j, int cols) {
    return i * cols + j;
}

int count_neighbors(const vector<int>& grid, int i, int j, int cols) {
    int total_cols = cols;
    return grid[idx(i-1, j-1, total_cols)] + grid[idx(i-1, j, total_cols)] + grid[idx(i-1, j+1, total_cols)] +
           grid[idx(i,   j-1, total_cols)] +                          grid[idx(i,   j+1, total_cols)] +
           grid[idx(i+1, j-1, total_cols)] + grid[idx(i+1, j, total_cols)] + grid[idx(i+1, j+1, total_cols)];
}

void initialize_grid(vector<int>& grid, int local_rows, int cols) {
    int total_cols = cols + 2;
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            grid[idx(i, j, total_cols)] = rand() % 2;
}

void copy_grid(vector<int>& dest, const vector<int>& src, int local_rows, int cols) {
    int total_cols = cols + 2;
    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            dest[idx(i, j, total_cols)] = src[idx(i, j, total_cols)];
}

void gather_and_print_global_grid(const vector<int>& local_grid, int local_rows, int cols, int total_cols, int rank, int size, int step) {
    vector<int> global_grid;
    if (rank == 0) global_grid.resize((local_rows * size) * cols);

    vector<int> sendbuf(local_rows * cols);
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            sendbuf[(i - 1) * cols + (j - 1)] = local_grid[idx(i, j, total_cols)];

    MPI_Gather(sendbuf.data(), local_rows * cols, MPI_INT,
               global_grid.data(), local_rows * cols, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "\n=== Paso " << step << " ===" << endl;
        for (int i = 0; i < local_rows * size; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << (global_grid[i * cols + j] == ALIVE ? 'O' : '.');
            }
            cout << '\n';
        }
        cout << flush;
        usleep(200000); // pausa visual
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4) {
        if (rank == 0) cout << "Uso: " << argv[0] << " filas columnas pasos\n";
        MPI_Finalize();
        return 1;
    }

    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    int steps = atoi(argv[3]);
    int local_rows = rows / size;

    int total_cols = cols + 2;
    int total_rows = local_rows + 2;

    srand(time(0) + rank);

    vector<int> current(total_rows * total_cols, DEAD);
    vector<int> next(total_rows * total_cols, DEAD);

    initialize_grid(current, local_rows, cols);

    int up = (rank - 1 + size) % size;
    int down = (rank + 1) % size;

    for (int step = 0; step < steps; ++step) {
        MPI_Sendrecv(&current[idx(1, 1, total_cols)], cols, MPI_INT, up, 0,
                     &current[idx(local_rows + 1, 1, total_cols)], cols, MPI_INT, down, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&current[idx(local_rows, 1, total_cols)], cols, MPI_INT, down, 1,
                     &current[idx(0, 1, total_cols)], cols, MPI_INT, up, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        #pragma omp parallel for
        for (int i = 0; i <= local_rows + 1; ++i) {
            current[idx(i, 0, total_cols)] = current[idx(i, cols, total_cols)];
            current[idx(i, cols + 1, total_cols)] = current[idx(i, 1, total_cols)];
        }

        #pragma omp parallel for collapse(2)
        for (int i = 1; i <= local_rows; ++i) {
            for (int j = 1; j <= cols; ++j) {
                int alive_neighbors = count_neighbors(current, i, j, total_cols);
                int& cell = next[idx(i, j, total_cols)];
                if (current[idx(i, j, total_cols)] == ALIVE) {
                    cell = (alive_neighbors == 2 || alive_neighbors == 3) ? ALIVE : DEAD;
                } else {
                    cell = (alive_neighbors == 3) ? ALIVE : DEAD;
                }
            }
        }

        copy_grid(current, next, local_rows, cols);
        gather_and_print_global_grid(current, local_rows, cols, total_cols, rank, size, step);
    }

    MPI_Finalize();
    return 0;
}
