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
const int ROWS = 20;        // Número de filas del tablero
const int COLS = 40;        // Número de columnas del tablero
const int GENERATIONS = 1000; // Número total de generaciones a simular
const char ALIVE = 'O';     // Carácter para representar células vivas
const char DEAD = ' ';      // Carácter para representar células muertas
const int SEED = 42;        // Semilla fija para inicialización reproducible

// ============================================================================
// FUNCIÓN PARA VISUALIZAR EL TABLERO EN CONSOLA
// ============================================================================
void printBoard(const std::vector<std::vector<int>>& board) {
    // Limpia la pantalla de la consola para mostrar la nueva generación
    system("clear"); // Cambia a "cls" si estás en Windows
    
    // Itera sobre cada fila del tablero
    for (const auto& row : board) {
        // Itera sobre cada celda en la fila actual
        for (int cell : row) {
            // Imprime 'O' si la celda está viva (valor 1), espacio en blanco si está muerta (valor 0)
            std::cout << (cell ? ALIVE : DEAD);
        }
        std::cout << "\n"; // Nueva línea al final de cada fila
    }
}

// ============================================================================
// FUNCIÓN PARA CONTAR VECINOS VIVOS DE UNA CÉLULA
// ============================================================================
int countLiveNeighbors(const std::vector<std::vector<int>>& board, int x, int y) {
    int count = 0;
    
    // Itera sobre las 8 posiciones vecinas (3x3 grid centrado en la celda)
    for (int dx = -1; dx <= 1; ++dx)      // Desplazamiento en dirección X
        for (int dy = -1; dy <= 1; ++dy)  // Desplazamiento en dirección Y
            if (!(dx == 0 && dy == 0)) {  // Excluye la celda central (0,0)
                // Calcula las coordenadas del vecino
                int nx = x + dx;
                int ny = y + dy;
                
                // Verifica que el vecino esté dentro de los límites del tablero
                if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS)
                    count += board[nx][ny]; // Suma 1 si está vivo, 0 si está muerto
            }
    return count;
}

// ============================================================================
// FUNCIÓN PARA CALCULAR LA SIGUIENTE GENERACIÓN
// ============================================================================
std::vector<std::vector<int>> nextGeneration(const std::vector<std::vector<int>>& board) {
    // Crea una copia del tablero actual para almacenar la nueva generación
    std::vector<std::vector<int>> newBoard = board;
    
    // Itera sobre cada celda del tablero
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j) {
            // Cuenta cuántos vecinos vivos tiene la celda actual
            int liveNeighbors = countLiveNeighbors(board, i, j);
            
            // Aplica las reglas del juego de la vida:
            if (board[i][j] == 1) {
                // Célula viva: sobrevive si tiene 2 o 3 vecinos vivos
                newBoard[i][j] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                // Célula muerta: nace si tiene exactamente 3 vecinos vivos
                newBoard[i][j] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    return newBoard;
}

// ============================================================================
// FUNCIÓN PARA INICIALIZAR EL TABLERO CON VALORES ALEATORIOS REPRODUCIBLES
// ============================================================================
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

// ============================================================================
// FUNCIÓN PRINCIPAL - SIMULACIÓN DEL JUEGO
// ============================================================================
int main() {
    // Inicializa el tablero con valores aleatorios reproducibles
    auto board = initializeBoard();

    // Bucle principal: simula cada generación
    for (int gen = 0; gen < GENERATIONS; ++gen) {
        // Muestra el número de la generación actual
        std::cout << "Generación: " << gen << "\n";
        
        // Visualiza el estado actual del tablero
        printBoard(board);
        
        // Calcula y actualiza el tablero para la siguiente generación
        board = nextGeneration(board);
        
        // Pausa de 200ms para hacer la visualización más lenta y observable
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
} 