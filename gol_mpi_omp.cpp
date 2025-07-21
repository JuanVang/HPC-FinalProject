// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN HÍBRIDA MPI + OpenMP 
// ============================================================================
// Este código implementa el juego de la vida usando:
// - MPI: Para paralelismo distribuido (múltiples procesos)
// - OpenMP: Para paralelismo compartido (múltiples hilos por proceso)
// 
// MEJORAS IMPLEMENTADAS DE LA VERSION ANTERIOR:
// 1. Distribución equitativa de filas (maneja casos donde rows % size != 0)
// 2. Comunicación no bloqueante (MPI_Isend/Irecv para superponer cálculo y comunicación)
// 3. Medición de tiempo precisa (MPI_Wtime + MPI_Barrier)
// 4. Recolección con MPI_Gatherv (maneja distribución desigual de filas)
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
    // TODOS los procesos usan la misma semilla para generar el mismo tablero
    srand(SEED);
    
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
// FUNCIÓN PARA CALCULAR FILAS POR PROCESO (DISTRIBUCIÓN EQUITATIVA)
// ============================================================================
int get_rows_for_process(int rank, int total_rows, int size) {
    int base_rows = total_rows / size;
    int extra = total_rows % size;
    return (rank < extra) ? base_rows + 1 : base_rows;
}

// ============================================================================
// FUNCIÓN PARA RECOLECTAR Y MOSTRAR EL GRID GLOBAL (MEJORADA CON MPI_Gatherv)
// ============================================================================
void gather_and_print_global_grid(const vector<int>& local_grid, int local_rows, int cols, int total_cols, 
                                 int rank, int size, int step, int total_rows) {
    vector<int> global_grid;
    vector<int> recvcounts(size);
    vector<int> displs(size);
    
    // Solo el proceso 0 necesita el grid completo para mostrar
    if (rank == 0) {
        global_grid.resize(total_rows * cols);
        
        // Calcula cuántos datos recibe de cada proceso
        int base_rows = total_rows / size;
        int extra = total_rows % size;
        int offset = 0;
        
        for (int i = 0; i < size; ++i) {
            int rows_for_process = (i < extra) ? base_rows + 1 : base_rows;
            recvcounts[i] = rows_for_process * cols;
            displs[i] = offset;
            offset += rows_for_process * cols;
        }
    }

    // Prepara los datos locales para enviar (excluye las columnas de borde)
    vector<int> sendbuf(local_rows * cols);
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            sendbuf[(i - 1) * cols + (j - 1)] = local_grid[idx(i, j, total_cols)];

    // MPI_Gatherv recolecta diferentes cantidades de datos de cada proceso
    MPI_Gatherv(sendbuf.data(), local_rows * cols, MPI_INT,
                global_grid.data(), recvcounts.data(), displs.data(), MPI_INT,
                0, MPI_COMM_WORLD);

    // Solo el proceso 0 imprime el resultado
    if (rank == 0) {
        cout << "\n=== Paso " << step << " ===" << endl;
        for (int i = 0; i < total_rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << (global_grid[i * cols + j] == ALIVE ? 'O' : ' ');
            }
            cout << '\n';
        }
        cout << flush;
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
    int num_threads = 0;  // 0 significa usar OMP_NUM_THREADS o valor por defecto
    
    // Parsea argumentos de línea de comandos
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--print") {
            print = true;
        } else if (arg == "--threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[i + 1]);
            i++;  // Saltar el siguiente argumento
        } else if (i + 2 < argc) {
            rows = std::stoi(argv[i]);
            cols = std::stoi(argv[i+1]);
            steps = std::stoi(argv[i+2]);
            i += 2;
        }
    }
    
    // Configuración de OpenMP
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
        if (rank == 0) {
            std::cout << "Configurados " << num_threads << " hilos OpenMP por proceso\n";
        }
    } else {
        // Usar valor por defecto o OMP_NUM_THREADS
        if (rank == 0) {
            std::cout << "Usando " << omp_get_max_threads() << " hilos OpenMP por proceso (por defecto)\n";
        }
    }
    
    // Distribución equitativa de filas
    int local_rows = get_rows_for_process(rank, rows, size);
    int total_cols = cols + 2;
    int total_rows = local_rows + 2;

    vector<int> current(total_rows * total_cols, DEAD);
    vector<int> next(total_rows * total_cols, DEAD);
    initialize_grid(current, local_rows, cols, rank);

    int up = (rank - 1 + size) % size;
    int down = (rank + 1) % size;

    // Sincronización antes de comenzar la medición de tiempo
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();
    
    for (int step = 0; step < steps; ++step) {
        // ========================================================================
        // FASE 1: COMUNICACIÓN NO BLOQUEANTE - INTERCAMBIO DE BORDES
        // ========================================================================
        MPI_Request requests[4];
        MPI_Status statuses[4];
        
        // Inicia comunicación no bloqueante
        MPI_Isend(&current[idx(1, 1, total_cols)], cols, MPI_INT, up, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(&current[idx(local_rows + 1, 1, total_cols)], cols, MPI_INT, down, 0, MPI_COMM_WORLD, &requests[1]);
        MPI_Isend(&current[idx(local_rows, 1, total_cols)], cols, MPI_INT, down, 1, MPI_COMM_WORLD, &requests[2]);
        MPI_Irecv(&current[idx(0, 1, total_cols)], cols, MPI_INT, up, 1, MPI_COMM_WORLD, &requests[3]);

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
        // FASE 3: CÁLCULO DE FILAS INTERNAS (NO DEPENDEN DE COMUNICACIÓN)
        // ========================================================================
        // Calcula las filas internas que no requieren las filas fantasma
        #pragma omp parallel for collapse(2)
        for (int i = 2; i < local_rows; ++i) {
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

        // ========================================================================
        // FASE 4: ESPERA FINALIZACIÓN DE COMUNICACIÓN Y CALCULA FILAS DE BORDE
        // ========================================================================
        MPI_Waitall(4, requests, statuses);

        // Calcula las filas de borde que requieren las filas fantasma
        #pragma omp parallel for collapse(2)
        for (int i = 1; i <= 1; ++i) {  // Primera fila
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
        
        #pragma omp parallel for collapse(2)
        for (int i = local_rows; i <= local_rows; ++i) {  // Última fila
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

        // ========================================================================
        // FASE 5: ACTUALIZACIÓN Y VISUALIZACIÓN
        // ========================================================================
        // Copia el grid de la siguiente generación al grid actual
        copy_grid(current, next, local_rows, cols);
        if (print) gather_and_print_global_grid(current, local_rows, cols, total_cols, rank, size, step, rows);
    }
    
    // Sincronización antes de finalizar la medición de tiempo
    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();
    
    if (rank == 0) {
        std::cout << "Tiempo de simulación: " << (t1-t0) << " segundos\n";
        std::cout << "Configuración: " << size << " procesos MPI, " << omp_get_max_threads() << " hilos OpenMP por proceso\n";
    }
    
    MPI_Finalize();
    return 0;
}
