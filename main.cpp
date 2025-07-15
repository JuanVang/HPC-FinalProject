#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

const int ROWS = 20;
const int COLS = 40;
const int GENERATIONS = 1000;
const char ALIVE = 'O';
const char DEAD = ' ';

// Función para imprimir el tablero
void printBoard(const std::vector<std::vector<int>>& board) {
    system("clear"); // Cambia a "cls" si estás en Windows
    for (const auto& row : board) {
        for (int cell : row) {
            std::cout << (cell ? ALIVE : DEAD);
        }
        std::cout << "\n";
    }
}

// Contar vecinos vivos
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

// Calcular la siguiente generación
std::vector<std::vector<int>> nextGeneration(const std::vector<std::vector<int>>& board) {
    std::vector<std::vector<int>> newBoard = board;
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

// Inicializa el tablero con una configuración básica (ej. glider)
std::vector<std::vector<int>> initializeBoard() {
    std::vector<std::vector<int>> board(ROWS, std::vector<int>(COLS, 0));
    // Glider (planeador)
    board[1][2] = 1;
    board[2][3] = 1;
    board[3][1] = 1;
    board[3][2] = 1;
    board[3][3] = 1;
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
