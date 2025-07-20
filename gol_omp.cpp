// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN OpenMP 
// ============================================================================
// Este código implementa el juego de la vida usando únicamente OpenMP para
// paralelismo a nivel de hilos, sin comunicación entre procesos (sin MPI).
// 
// MEJORAS IMPLEMENTADAS DE LA VERSION ANTERIOR:
// 1. Optimización de cache (acceso por filas para mejor localidad)
// 2. Medición de tiempo precisa con omp_get_wtime()
// 3. Configuración optimizada de OpenMP (chunk size, scheduling)
// 4. Reducción de overhead de sincronización
// ============================================================================

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <omp.h>
#include <cstdlib>
#include <ctime>

const char ALIVE = 'O';
const char DEAD = ' ';
const int SEED = 42;

void printBoard(const std::vector<std::vector<int>>& board, int rows, int cols) {
    system("clear");
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << (board[i][j] ? ALIVE : DEAD);
        }
        std::cout << "\n";
    }
}

int countLiveNeighbors(const std::vector<std::vector<int>>& board, int x, int y, int rows, int cols) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (!(dx == 0 && dy == 0)) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < rows && ny >= 0 && ny < cols)
                    count += board[nx][ny];
            }
    return count;
}

std::vector<std::vector<int>> nextGeneration(const std::vector<std::vector<int>>& board, int rows, int cols) {
    std::vector<std::vector<int>> newBoard = board;
    
    // Optimización: usar static scheduling con chunk size para mejor balance de carga
    // y reducir overhead de sincronización
    #pragma omp parallel for collapse(2) schedule(static, 16)
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) {
            int liveNeighbors = countLiveNeighbors(board, i, j, rows, cols);
            if (board[i][j] == 1) {
                newBoard[i][j] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                newBoard[i][j] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    return newBoard;
}

std::vector<std::vector<int>> initializeBoard(int rows, int cols) {
    srand(SEED);
    std::vector<std::vector<int>> board(rows, std::vector<int>(cols, 0));
    
    // Paralelizar la inicialización para mejor rendimiento
    #pragma omp parallel for collapse(2) schedule(static, 16)
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            board[i][j] = (rand() % 100 < 20) ? 1 : 0;
    
    return board;
}

int main(int argc, char* argv[]) {
    int rows = 10, cols = 10, generations = 10;
    bool print = false;
    int num_threads = 0;  // 0 significa usar OMP_NUM_THREADS o valor por defecto
    
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
            generations = std::stoi(argv[i+2]);
            i += 2;
        }
    }
    
    // Configuración optimizada de OpenMP
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
        std::cout << "Configurados " << num_threads << " hilos OpenMP\n";
    } else {
        // Establecer número de hilos si no está definido
        if (getenv("OMP_NUM_THREADS") == nullptr) {
            omp_set_num_threads(omp_get_max_threads());
        }
        std::cout << "Usando " << omp_get_max_threads() << " hilos OpenMP (por defecto)\n";
    }
    
    // Configurar scheduling para mejor rendimiento
    omp_set_schedule(omp_sched_static, 16);
    
    auto board = initializeBoard(rows, cols);
    
    // Medición de tiempo precisa
    double t0 = omp_get_wtime();
    
    for (int gen = 0; gen < generations; ++gen) {
        if (print) {
            std::cout << "Generación: " << gen << "\n";
            printBoard(board, rows, cols);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        board = nextGeneration(board, rows, cols);
    }
    
    double t1 = omp_get_wtime();
    std::cout << "Tiempo de simulación: " << (t1-t0) << " segundos\n";
    std::cout << "Hilos utilizados: " << omp_get_max_threads() << "\n";
    
    return 0;
} 