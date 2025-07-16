#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

// ============================================================================
// CONFIGURACIÓN GLOBAL DEL JUEGO
// ============================================================================
const int ROWS = 20;        // Número de filas del tablero
const int COLS = 40;        // Número de columnas del tablero
const int GENERATIONS = 1000; // Número total de generaciones a simular
const char ALIVE = 'O';     // Carácter para representar células vivas
const char DEAD = ' ';      // Carácter para representar células muertas

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
// FUNCIÓN PARA INICIALIZAR EL TABLERO CON UN PATRÓN ESPECÍFICO
// ============================================================================
std::vector<std::vector<int>> initializeBoard() {
    // Crea un tablero vacío (todas las células muertas)
    std::vector<std::vector<int>> board(ROWS, std::vector<int>(COLS, 0));
    
    // Configura un "glider" (planeador) - un patrón que se mueve diagonalmente
    // El glider es un patrón clásico del juego de la vida que se desplaza
    board[1][2] = 1;  // Primera fila del glider
    board[2][3] = 1;  // Segunda fila del glider
    board[3][1] = 1;  // Tercera fila del glider
    board[3][2] = 1;  // (parte izquierda)
    board[3][3] = 1;  // (parte derecha)
    
    return board;
}

// ============================================================================
// FUNCIÓN PRINCIPAL - SIMULACIÓN DEL JUEGO
// ============================================================================
int main() {
    // Inicializa el tablero con el patrón glider
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
