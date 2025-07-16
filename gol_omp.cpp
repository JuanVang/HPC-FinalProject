// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN SOLO OpenMP
// ============================================================================
// Este código implementa el juego de la vida usando únicamente OpenMP para
// paralelismo a nivel de hilos, sin comunicación entre procesos (sin MPI).
// ============================================================================

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <omp.h>
#include <cstdlib>
#include <ctime>

const int ROWS = 20;        // Número de filas del tablero
const int COLS = 40;        // Número de columnas del tablero
const int GENERATIONS = 1000; // Número total de generaciones a simular
const char ALIVE = 'O';     // Carácter para representar células vivas
const char DEAD = ' ';      // Carácter para representar células muertas
const int SEED = 42;        // Semilla fija para inicialización reproducible

// Imprime el tablero en consola
void printBoard(const std::vector<std::vector<int>>& board) {
    system("clear"); // Cambia a "cls" si estás en Windows
    for (const auto& row : board) {
        for (int cell : row) {
            std::cout << (cell ? ALIVE : DEAD);
        }
        std::cout << "\n";
    }
}

// Cuenta los vecinos vivos de una celda
int countLiveNeighbors(const std::vector<std::vector<int>>& board, int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (!(dx == 0 && dy == 0)) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS)
                    count += board[nx][ny];
            }
    return count;
}

// Calcula la siguiente generación usando OpenMP
std::vector<std::vector<int>> nextGeneration(const std::vector<std::vector<int>>& board) {
    std::vector<std::vector<int>> newBoard = board;
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j) {
            int liveNeighbors = countLiveNeighbors(board, i, j);
            if (board[i][j] == 1) {
                newBoard[i][j] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                newBoard[i][j] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    return newBoard;
}

// Inicializa el tablero con valores aleatorios reproducibles
std::vector<std::vector<int>> initializeBoard() {
    // Inicializa el generador de números aleatorios con semilla fija
    srand(SEED);
    
    // Crea un tablero vacío (todas las células muertas)
    std::vector<std::vector<int>> board(ROWS, std::vector<int>(COLS, 0));
    
    // Inicializa el tablero con valores aleatorios (0 o 1, 20% vivas)
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            board[i][j] = (rand() % 100 < 20) ? 1 : 0;  // 20% probabilidad de estar viva
        }
    }
    
    return board;
}

int main() {
    auto board = initializeBoard();
    for (int gen = 0; gen < GENERATIONS; ++gen) {
        std::cout << "Generación: " << gen << "\n";
        printBoard(board);
        board = nextGeneration(board);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return 0;
} 