// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN HÍBRIDA MPI + OpenMP
// ============================================================================
// Este código implementa el juego de la vida usando:
// - MPI: Para paralelismo distribuido (múltiples procesos)
// - OpenMP: Para paralelismo compartido (múltiples hilos por proceso)
// ============================================================================

#include <mpi.h>      // Para comunicación entre procesos (MPI)
#include <omp.h>      // Para paralelismo de hilos (OpenMP)
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h> // para usleep
#include <string>

// ============================================================================
// CONSTANTES Y DEFINICIONES
// ============================================================================
#define ALIVE 1    // Valor que representa una célula viva
#define DEAD  0    // Valor que representa una célula muerta
#define SEED 42    // Semilla fija para inicialización reproducible

using namespace std;

// ============================================================================
// FUNCIÓN AUXILIAR PARA CALCULAR ÍNDICE EN ARRAY UNIDIMENSIONAL
// ============================================================================
// Convierte coordenadas 2D (i,j) a índice 1D en un array
// total_cols incluye las columnas de borde (ghost cells)
int idx(int i, int j, int cols) {
    return i * cols + j;
}

// ============================================================================
// FUNCIÓN PARA CONTAR VECINOS VIVOS DE UNA CÉLULA
// ============================================================================
// Esta función suma los valores de las 8 células vecinas
// Asume que el grid tiene bordes (ghost cells) para manejar condiciones de frontera
int count_neighbors(const vector<int>& grid, int i, int j, int cols) {
    int total_cols = cols;
    // Suma directa de los 8 vecinos (3x3 grid menos la celda central)
    return grid[idx(i-1, j-1, total_cols)] + grid[idx(i-1, j, total_cols)] + grid[idx(i-1, j+1, total_cols)] +
           grid[idx(i,   j-1, total_cols)] +                          grid[idx(i,   j+1, total_cols)] +
           grid[idx(i+1, j-1, total_cols)] + grid[idx(i+1, j, total_cols)] + grid[idx(i+1, j+1, total_cols)];
}

// ============================================================================
// FUNCIÓN PARA INICIALIZAR EL GRID CON VALORES ALEATORIOS REPRODUCIBLES
// ============================================================================
void initialize_grid(vector<int>& grid, int local_rows, int cols, int rank) {
    int total_cols = cols + 2;  // +2 para incluir las columnas de borde
    
    // Inicializa el generador de números aleatorios con semilla fija
    // Usamos la semilla base + rank para que cada proceso tenga una secuencia diferente
    // pero reproducible
    srand(SEED + rank);
    
    // Inicializa solo las células internas (no los bordes)
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            grid[idx(i, j, total_cols)] = (rand() % 100 < 20) ? 1 : 0;  // 20% probabilidad de estar viva
}

// ============================================================================
// FUNCIÓN PARA COPIAR GRID USANDO OpenMP
// ============================================================================
void copy_grid(vector<int>& dest, const vector<int>& src, int local_rows, int cols) {
    int total_cols = cols + 2;
    // Directiva OpenMP para paralelizar el bucle anidado
    // collapse(2) permite que OpenMP distribuya las iteraciones de ambos bucles
    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            dest[idx(i, j, total_cols)] = src[idx(i, j, total_cols)];
}

// ============================================================================
// FUNCIÓN PARA RECOLECTAR Y MOSTRAR EL GRID GLOBAL
// ============================================================================
void gather_and_print_global_grid(const vector<int>& local_grid, int local_rows, int cols, int total_cols, int rank, int size, int step) {
    vector<int> global_grid;
    // Solo el proceso 0 necesita el grid completo para mostrar
    if (rank == 0) global_grid.resize((local_rows * size) * cols);

    // Prepara los datos locales para enviar (excluye las columnas de borde)
    vector<int> sendbuf(local_rows * cols);
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            sendbuf[(i - 1) * cols + (j - 1)] = local_grid[idx(i, j, total_cols)];

    // MPI_Gather recolecta datos de todos los procesos en el proceso 0
    MPI_Gather(sendbuf.data(), local_rows * cols, MPI_INT,
               global_grid.data(), local_rows * cols, MPI_INT,
               0, MPI_COMM_WORLD);

    // Solo el proceso 0 imprime el resultado
    if (rank == 0) {
        cout << "\n=== Paso " << step << " ===" << endl;
        for (int i = 0; i < local_rows * size; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << (global_grid[i * cols + j] == ALIVE ? 'O' : '.');
            }
            cout << '\n';
        }
        cout << flush;
        usleep(200000); // Pausa de 200ms para visualización
    }
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================
int main(int argc, char** argv) {
    // Inicializa MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows = 10, cols = 10, steps = 10;
    bool print = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--print") print = true;
        else if (i + 2 < argc) {
            rows = std::stoi(argv[i]);
            cols = std::stoi(argv[i+1]);
            steps = std::stoi(argv[i+2]);
            i += 2;
        }
    }
    int local_rows = rows / size;
    int total_cols = cols + 2;
    int total_rows = local_rows + 2;

    vector<int> current(total_rows * total_cols, DEAD);
    vector<int> next(total_rows * total_cols, DEAD);
    initialize_grid(current, local_rows, cols, rank);

    int up = (rank - 1 + size) % size;
    int down = (rank + 1) % size;

    double t0 = 0, t1 = 0;
    if (rank == 0) t0 = omp_get_wtime();
    for (int step = 0; step < steps; ++step) {
        // ========================================================================
        // FASE 1: COMUNICACIÓN MPI - INTERCAMBIO DE BORDES
        // ========================================================================
        // Envía la primera fila de datos al proceso superior y recibe del inferior
        MPI_Sendrecv(&current[idx(1, 1, total_cols)], cols, MPI_INT, up, 0,
                     &current[idx(local_rows + 1, 1, total_cols)], cols, MPI_INT, down, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Envía la última fila de datos al proceso inferior y recibe del superior
        MPI_Sendrecv(&current[idx(local_rows, 1, total_cols)], cols, MPI_INT, down, 1,
                     &current[idx(0, 1, total_cols)], cols, MPI_INT, up, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // ========================================================================
        // FASE 2: APLICACIÓN DE CONDICIONES DE FRONTERA PERIÓDICAS
        // ========================================================================
        // Copia las columnas de borde para simular condiciones periódicas
        #pragma omp parallel for
        for (int i = 0; i <= local_rows + 1; ++i) {
            current[idx(i, 0, total_cols)] = current[idx(i, cols, total_cols)];        // Borde izquierdo = borde derecho
            current[idx(i, cols + 1, total_cols)] = current[idx(i, 1, total_cols)];    // Borde derecho = borde izquierdo
        }

        // ========================================================================
        // FASE 3: CÁLCULO DE LA NUEVA GENERACIÓN CON OpenMP
        // ========================================================================
        // Paraleliza el cálculo de la nueva generación usando OpenMP
        #pragma omp parallel for collapse(2)
        for (int i = 1; i <= local_rows; ++i) {
            for (int j = 1; j <= cols; ++j) {
                // Cuenta vecinos vivos
                int alive_neighbors = count_neighbors(current, i, j, total_cols);
                // Referencia a la celda en el grid de la siguiente generación
                int& cell = next[idx(i, j, total_cols)];
                // Aplica las reglas del juego de la vida
                if (current[idx(i, j, total_cols)] == ALIVE) {
                    // Célula viva: sobrevive si tiene 2 o 3 vecinos vivos
                    cell = (alive_neighbors == 2 || alive_neighbors == 3) ? ALIVE : DEAD;
                } else {
                    // Célula muerta: nace si tiene exactamente 3 vecinos vivos
                    cell = (alive_neighbors == 3) ? ALIVE : DEAD;
                }
            }
        }

        // ========================================================================
        // FASE 4: ACTUALIZACIÓN Y VISUALIZACIÓN
        // ========================================================================
        // Copia el grid de la siguiente generación al grid actual
        copy_grid(current, next, local_rows, cols);
        if (print) gather_and_print_global_grid(current, local_rows, cols, total_cols, rank, size, step);
    }
    if (rank == 0) {
        t1 = omp_get_wtime();
        std::cout << "Tiempo de simulación: " << (t1-t0) << " segundos\n";
    }
    MPI_Finalize();
    return 0;
}
