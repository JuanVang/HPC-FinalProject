// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN HÍBRIDA MPI + OpenMP 
// ============================================================================
#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <fstream>
#include <random>
#include <thread>
#include <chrono>

#define ALIVE 1
#define DEAD  0
#define SEED 42

// Función auxiliar para índice 1D
int idx(int i, int j, int cols) { return i * cols + j; }

// ============================================================================
// FUNCIÓN PARA LIMPIAR LA CONSOLA (ANIMACIÓN)
// ============================================================================
void clearScreen() {
    // ANSI escape code para limpiar pantalla y mover cursor al inicio
    std::cout << "\033[2J\033[H";
}

// ============================================================================
// FUNCIÓN PARA INICIALIZAR EL TABLERO CON PATRÓN ALEATORIO ESTÁNDAR (GLOBAL)
// ============================================================================
// Esta función solo se usa en el proceso raíz para inicializar el tablero global
std::vector<int> initializeBoard(int rows, int cols, double prob = 0.2, int seed = 42) {
    std::vector<int> board(rows * cols, 0);
    std::mt19937 gen(seed);
    std::bernoulli_distribution dist(prob);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            board[i * cols + j] = dist(gen) ? 1 : 0;
    return board;
}

// ============================================================================
// FUNCIÓN PARA INICIALIZAR UN GLIDER EN LA ESQUINA SUPERIOR IZQUIERDA
// ============================================================================
std::vector<int> initializeGliderBoard(int rows, int cols) {
    std::vector<int> board(rows * cols, 0);
    if (rows < 3 || cols < 3) return board;
    board[idx(0, 1, cols)] = 1;
    board[idx(1, 2, cols)] = 1;
    board[idx(2, 0, cols)] = 1;
    board[idx(2, 1, cols)] = 1;
    board[idx(2, 2, cols)] = 1;
    return board;
}

// Visualización
void print_global_grid(const std::vector<int>& local_grid, int local_rows, int cols, int total_cols, int rank, int size, int step, int total_rows) {
    std::vector<int> global_grid;
    std::vector<int> recvcounts(size), displs(size);
    if (rank == 0) {
        global_grid.resize(total_rows * cols);
        int base_rows = total_rows / size, extra = total_rows % size, offset = 0;
        for (int i = 0; i < size; ++i) {
            int rows_for_process = (i < extra) ? base_rows + 1 : base_rows;
            recvcounts[i] = rows_for_process * cols;
            displs[i] = offset;
            offset += rows_for_process * cols;
        }
    }
    std::vector<int> sendbuf(local_rows * cols);
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            sendbuf[(i-1)*cols + (j-1)] = local_grid[idx(i, j, total_cols)];
    MPI_Gatherv(sendbuf.data(), local_rows*cols, MPI_INT,
                global_grid.data(), recvcounts.data(), displs.data(), MPI_INT,
                0, MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << "\n=== Paso " << step << " ===\n";
        for (int i = 0; i < total_rows; ++i) {
            for (int j = 0; j < cols; ++j)
                std::cout << (global_grid[i*cols + j] == ALIVE ? 'O' : ' ');
            std::cout << '\n';
        }
        std::cout << std::flush;
    }
}

// Conteo de vecinos
int count_neighbors(const std::vector<int>& grid, int i, int j, int total_cols) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (!(dx == 0 && dy == 0)) {
                // Acceso directo a ghost rows/columns, que ya contienen la información toroidal
                count += grid[idx(i + dx, j + dy, total_cols)];
            }
    return count;
}

// Copia de grilla
void copy_grid(std::vector<int>& dest, const std::vector<int>& src, int local_rows, int cols, int total_cols) {
    
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            dest[idx(i, j, total_cols)] = src[idx(i, j, total_cols)];
}

// Cálculo de filas por proceso
int get_rows_for_process(int rank, int total_rows, int size) {
    int base_rows = total_rows / size, extra = total_rows % size;
    return (rank < extra) ? base_rows + 1 : base_rows;
}

void print_local_debug(const std::vector<int>& grid, int local_rows, int cols, int total_cols, int rank, int step) {
    std::cout << "[DEBUG] Proceso " << rank << ", paso " << step << ":\n";
    for (int i = 0; i < local_rows + 2; ++i) {
        for (int j = 0; j < cols + 2; ++j) {
            std::cout << (grid[idx(i, j, total_cols)] == ALIVE ? 'O' : (grid[idx(i, j, total_cols)] == DEAD ? '.' : '?'));
        }
        std::cout << "\n";
    }
    std::cout << std::flush;
}

int main(int argc, char** argv) {
    // I. Inicializar entorno distribuido
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // II. Parsear argumentos
    int rows = 10, cols = 10, steps = 10, num_threads = 0;
    bool print = false;
    bool anim = false;
    std::string savefile = "";
    double prob = 0.2;
    int seed = 42;
    bool debug = false;
    std::string preset = "";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--print") print = true;
        else if (arg == "--anim") anim = true;
        else if (arg == "--threads" && i+1 < argc) { num_threads = std::stoi(argv[++i]); }
        else if (arg == "--preset" && i+1 < argc) { preset = argv[++i]; }
        else if (arg == "--save" && i+1 < argc) { savefile = argv[++i]; }
        else if (arg == "--prob" && i+1 < argc) { prob = std::stod(argv[++i]); }
        else if (arg == "--seed" && i+1 < argc) { seed = std::stoi(argv[++i]); }
        else if (arg == "--debug") debug = true;
        else if (i+2 < argc) { rows = std::stoi(argv[i]); cols = std::stoi(argv[i+1]); steps = std::stoi(argv[i+2]); i += 2; }
    }

    // III. Verificar parámetros
    if (rows < size || cols < 1 || steps < 1) {
        if (rank == 0) std::cerr << "Error: parámetros insuficientes." << std::endl;
        MPI_Finalize(); return 1;
    }

    // V. Determinar filas locales
    int local_rows = get_rows_for_process(rank, rows, size);
    int total_cols = cols + 2;
    int total_rows = local_rows + 2;
    int global_row_offset = 0;
    for (int i = 0; i < rank; ++i) global_row_offset += get_rows_for_process(i, rows, size);

    // VI. Reservar espacio para grillas
    std::vector<int> current(total_rows * total_cols, DEAD);
    std::vector<int> next(total_rows * total_cols, DEAD);

    // VII. Inicializar grilla (ahora global y coherente)
    std::vector<int> global_board;
    if (rank == 0) {
        if (preset == "glider") {
            global_board = initializeGliderBoard(rows, cols);
        } else {
            global_board = initializeBoard(rows, cols, prob, seed);
        }
    }
    // Cada proceso recibe su bloque de filas (local_rows * cols)
    std::vector<int> local_block(local_rows * cols, 0);
    MPI_Scatter(
        global_board.data(), local_rows * cols, MPI_INT,
        local_block.data(), local_rows * cols, MPI_INT,
        0, MPI_COMM_WORLD
    );
    // Ahora copio local_block a la grilla local (con halo)
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 1; j <= cols; ++j)
            current[idx(i, j, total_cols)] = local_block[(i - 1) * cols + (j - 1)];

    // VIII-IX. Determinar vecinos
    int up = (rank - 1 + size) % size;
    int down = (rank + 1) % size;

    // Sincronización antes de medir tiempo
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    // X. Simulación
    for (int step = 0; step < steps; ++step) {
        if (anim) {
            clearScreen();
            print_global_grid(current, local_rows, cols, total_cols, rank, size, step, rows);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else if (print) {
            print_global_grid(current, local_rows, cols, total_cols, rank, size, step, rows);
        }

        // XA. Ghost rows (comunicación)
        MPI_Request reqs[4];
        MPI_Isend(&current[idx(1, 1, total_cols)], cols, MPI_INT, up, 0, MPI_COMM_WORLD, &reqs[0]);
        MPI_Irecv(&current[idx(local_rows+1, 1, total_cols)], cols, MPI_INT, down, 0, MPI_COMM_WORLD, &reqs[1]);
        MPI_Isend(&current[idx(local_rows, 1, total_cols)], cols, MPI_INT, down, 1, MPI_COMM_WORLD, &reqs[2]);
        MPI_Irecv(&current[idx(0, 1, total_cols)], cols, MPI_INT, up, 1, MPI_COMM_WORLD, &reqs[3]);

        // Esperar comunicación antes de calcular filas de borde
        MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);

        // XB. Ghost columns (periódicas) - MOVER AQUÍ
       
        for (int i = 0; i <= local_rows+1; ++i) {
            current[idx(i, 0, total_cols)] = current[idx(i, cols, total_cols)];
            current[idx(i, cols+1, total_cols)] = current[idx(i, 1, total_cols)];
        }

        // Depuración: imprimir subtablero local con ghost rows/columns
        if (debug) {
            MPI_Barrier(MPI_COMM_WORLD);
            print_local_debug(current, local_rows, cols, total_cols, rank, step);
            MPI_Barrier(MPI_COMM_WORLD);
        }

        // XC. Visualización
        if (print) print_global_grid(current, local_rows, cols, total_cols, rank, size, step, rows);

        // XD. Cálculo de la siguiente generación
       
        for (int i = 1; i <= local_rows; ++i) {
            for (int j = 1; j <= cols; ++j) {
                int alive_neighbors = count_neighbors(current, i, j, total_cols);
                int& cell = next[idx(i, j, total_cols)];
                if (current[idx(i, j, total_cols)] == ALIVE)
                    cell = (alive_neighbors == 2 || alive_neighbors == 3) ? ALIVE : DEAD;
                else
                    cell = (alive_neighbors == 3) ? ALIVE : DEAD;
            }
        }

        // XE. Copiar next a current
        copy_grid(current, next, local_rows, cols, total_cols);
    }

    // Recolectar la última generación en el proceso 0 SOLO si se usa --save
    if (!savefile.empty()) {
        std::vector<int> global_grid;
        std::vector<int> recvcounts(size), displs(size);
        if (rank == 0) {
            global_grid.resize(rows * cols);
            int base_rows = rows / size, extra = rows % size, offset = 0;
            for (int i = 0; i < size; ++i) {
                int rows_for_process = (i < extra) ? base_rows + 1 : base_rows;
                recvcounts[i] = rows_for_process * cols;
                displs[i] = offset;
                offset += rows_for_process * cols;
            }
            // Depuración: imprimir recvcounts y displs
            std::cout << "[DEBUG] recvcounts: ";
            for (int i = 0; i < size; ++i) std::cout << recvcounts[i] << " ";
            std::cout << "\n[DEBUG] displs: ";
            for (int i = 0; i < size; ++i) std::cout << displs[i] << " ";
            std::cout << std::endl;
            std::cout << "[DEBUG] global_grid size: " << global_grid.size() << std::endl;
        }
        std::vector<int> sendbuf(local_rows * cols);
        for (int i = 1; i <= local_rows; ++i)
            for (int j = 1; j <= cols; ++j)
                sendbuf[(i-1)*cols + (j-1)] = current[idx(i, j, total_cols)];
        MPI_Gatherv(sendbuf.data(), local_rows*cols, MPI_INT,
                    global_grid.data(), recvcounts.data(), displs.data(), MPI_INT,
                    0, MPI_COMM_WORLD);
        if (rank == 0) {
            std::ofstream fout(savefile);
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j)
                    fout << (global_grid[i*cols + j] ? 'O' : ' ');
                fout << '\n';
            }
            fout.close();
        }
    }

    // Sincronización y tiempo final
    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();
    if (rank == 0) {
        std::cout << "Tiempo de simulación: " << (t1-t0) << " segundos\n";
    }

    // XI. Liberar estructuras (automático por vector)
    // XII. Finalizar entorno distribuido
    MPI_Finalize();
    return 0;
}
