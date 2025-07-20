// ============================================================================
// JUEGO DE LA VIDA - IMPLEMENTACIÓN SECUENCIAL
// ============================================================================
// Este código implementa el juego de la vida de forma secuencial (sin paralelismo)
// ============================================================================

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>

// ============================================================================
// CONFIGURACIÓN GLOBAL DEL JUEGO
// ============================================================================
const char ALIVE = 'O';     // Carácter para representar células vivas
const char DEAD = ' ';      // Carácter para representar células muertas
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
    // Limpia la pantalla de la consola para mostrar la nueva generación
    system("clear"); // Cambiar a "cls" en Windows 
    
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
// FUNCIÓN PARA CONTAR VECINOS VIVOS DE UNA CÉLULA
// ============================================================================
int countLiveNeighbors(const std::vector<int>& board, int x, int y, int rows, int cols) {
    int count = 0;
    
    // Itera sobre las 8 posiciones vecinas (3x3 grid centrado en la celda)
    for (int dx = -1; dx <= 1; ++dx)      // Desplazamiento en dirección X
        for (int dy = -1; dy <= 1; ++dy)  // Desplazamiento en dirección Y
            if (!(dx == 0 && dy == 0)) {  // Excluye la celda central (0,0)
                // Calcula las coordenadas del vecino
                int nx = x + dx;
                int ny = y + dy;
                
                // Verifica que el vecino esté dentro de los límites del tablero
                if (nx >= 0 && nx < rows && ny >= 0 && ny < cols)
                    count += board[idx(nx, ny, cols)]; // Suma 1 si está vivo, 0 si está muerto
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
// FUNCIÓN PARA INICIALIZAR EL TABLERO CON VALORES ALEATORIOS REPRODUCIBLES
// ============================================================================
std::vector<int> initializeBoard(int rows, int cols) {
    // Inicializa el generador de números aleatorios con semilla fija
    srand(SEED);
    
    // Crea un tablero vacío (todas las células muertas)
    std::vector<int> board(rows * cols, 0);
    
    // Inicializa el tablero con valores aleatorios (0 o 1, 20% vivas)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            board[idx(i, j, cols)] = (rand() % 100 < 20) ? 1 : 0;  // 20% probabilidad de estar viva
        }
    }
    
    return board;
}

// ============================================================================
// FUNCIÓN PRINCIPAL - SIMULACIÓN DEL JUEGO
// ============================================================================
int main(int argc, char* argv[]) {
    // Inicializa el tablero con valores aleatorios reproducibles
    int rows = 10, cols = 10, generations = 10;
    bool print = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--print") print = true;
        else if (i + 2 < argc) {
            rows = std::stoi(argv[i]);
            cols = std::stoi(argv[i+1]);
            generations = std::stoi(argv[i+2]);
            i += 2;
        }
    }
    auto board = initializeBoard(rows, cols);
    
    // Medición de tiempo usando std::chrono (sin dependencia de OpenMP)
    auto t0 = std::chrono::high_resolution_clock::now();
    
    // Bucle principal: simula cada generación
    for (int gen = 0; gen < generations; ++gen) {
        // Muestra el número de la generación actual
        if (print) {
            std::cout << "Generación: " << gen << "\n";
            printBoard(board, rows, cols);
            // Pausa de 200ms para hacer la visualización más lenta y observable
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        // Calcula y actualiza el tablero para la siguiente generación
        board = nextGeneration(board, rows, cols);
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    std::cout << "Tiempo de simulación: " << duration.count() / 1000000.0 << " segundos\n";
    return 0;
} 