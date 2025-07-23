// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN SECUENCIAL
// ============================================================================
// Este código implementa el juego de la vida de forma secuencial (sin paralelismo)
// ============================================================================

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <random>

// ============================================================================
// CONFIGURACIÓN GLOBAL DEL JUEGO
// ============================================================================
const char ALIVE = 'O';     // Carácter para representar células vivas
const char DEAD = '.';      // Carácter para representar células muertas (espacio)
const int SEED = 42;        // Semilla fija para inicialización reproducible

// ============================================================================
// FUNCIÓN AUXILIAR PARA CALCULAR ÍNDICE EN ARRAY UNIDIMENSIONAL
// ============================================================================
// Convierte coordenadas 2D (i,j) a índice 1D en un array
int idx(int i, int j, int cols) {
    return i * cols + j;
}

// ============================================================================
// FUNCIÓN PARA VISUALIZAR EL TABLERO EN CONSOLA
// ============================================================================
void printBoard(const std::vector<int>& board, int rows, int cols) {
    // Itera sobre cada fila del tablero
    for (int i = 0; i < rows; ++i) {
        // Itera sobre cada celda en la fila actual
        for (int j = 0; j < cols; ++j) {
            // Imprime 'O' si la celda está viva (valor 1), espacio en blanco si está muerta (valor 0)
            std::cout << (board[idx(i, j, cols)] ? ALIVE : DEAD);
        }
        std::cout << "\n"; // Nueva línea al final de cada fila
    }
}

// ============================================================================
// FUNCIÓN PARA LIMPIAR LA CONSOLA (ANIMACIÓN)
// ============================================================================
void clearScreen() {
    // ANSI escape code para limpiar pantalla y mover cursor al inicio
    std::cout << "\033[2J\033[H";
}

// ============================================================================
// FUNCIÓN PARA CONTAR VECINOS VIVOS DE UNA CÉLULA
// ============================================================================
int countLiveNeighbors(const std::vector<int>& board, int x, int y, int rows, int cols) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (!(dx == 0 && dy == 0)) {
                int nx = (x + dx + rows) % rows;
                int ny = (y + dy + cols) % cols;
                count += board[idx(nx, ny, cols)];
            }
    return count;
}

// ============================================================================
// FUNCIÓN PARA CALCULAR LA SIGUIENTE GENERACIÓN
// ============================================================================
std::vector<int> nextGeneration(const std::vector<int>& board, int rows, int cols) {
    std::vector<int> newBoard = board;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) {
            int liveNeighbors = countLiveNeighbors(board, i, j, rows, cols);
            if (board[idx(i, j, cols)] == 1) {
                newBoard[idx(i, j, cols)] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                newBoard[idx(i, j, cols)] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    return newBoard;
}

// ============================================================================
// FUNCIÓN PARA INICIALIZAR EL TABLERO CON PATRÓN ALEATORIO ESTÁNDAR
// ============================================================================
std::vector<int> initializeBoard(int rows, int cols, double prob = 0.2, int seed = 42) {
    std::vector<int> board(rows * cols, 0);
    std::mt19937 gen(seed);
    std::bernoulli_distribution dist(prob);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            board[idx(i, j, cols)] = dist(gen) ? 1 : 0;
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

// ============================================================================
// FUNCIÓN PRINCIPAL - SIMULACIÓN DEL JUEGO
// ============================================================================
int main(int argc, char* argv[]) {
    // Inicializa el tablero con valores aleatorios reproducibles
    int rows = 10, cols = 10, generations = 10;
    bool print = false;
    bool anim = false;
    std::string savefile = "";
    double prob = 0.2;
    int seed = 42;
    std::string preset = "";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--print") print = true;
        else if (arg == "--anim") anim = true;
        else if (arg == "--preset" && i + 1 < argc) { preset = argv[++i]; }
        else if (arg == "--save" && i + 1 < argc) { savefile = argv[++i]; }
        else if (arg == "--prob" && i + 1 < argc) { prob = std::stod(argv[++i]); }
        else if (arg == "--seed" && i + 1 < argc) { seed = std::stoi(argv[++i]); }
        else if (i + 2 < argc) {
            rows = std::stoi(argv[i]);
            cols = std::stoi(argv[i+1]);
            generations = std::stoi(argv[i+2]);
            i += 2;
        }
    }
    std::vector<int> board;
    if (preset == "glider") {
        board = initializeGliderBoard(rows, cols);
    } else {
        board = initializeBoard(rows, cols, prob, seed);
    }
    
    // Medición de tiempo usando std::chrono (sin dependencia de OpenMP)
    auto t0 = std::chrono::high_resolution_clock::now();
    
    // Bucle principal: simula cada generación
    for (int gen = 0; gen < generations; ++gen) {
        if (anim) {
            clearScreen();
            printBoard(board, rows, cols);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else if (print) {
            std::cout << "Generación: " << gen << "\n";
            printBoard(board, rows, cols);
        }
        board = nextGeneration(board, rows, cols);
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    std::cout << "Tiempo de simulación: " << duration.count() / 1000000.0 << " segundos\n";
    // Guardar la última generación si se solicita
    if (!savefile.empty()) {
        std::ofstream fout(savefile);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j)
                fout << (board[idx(i, j, cols)] ? 'O' : ' ');
            fout << '\n';
        }
        fout.close();
    }
    return 0;
} 