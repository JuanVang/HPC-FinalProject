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
#include "mpi.h"

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
void printBoard(const std::vector<int>& board, int rows, int cols, const std::vector<int> &rows_per_proc, const int &pid) {
    // Itera sobre cada fila del tablero
    //system("clear");

    for (int i = 0; i < rows_per_proc[pid]; ++i) {
        // Itera sobre cada celda en la fila actual
        for (int j = 0; j < cols; ++j) {
            // Imprime 'O' si la celda está viva (valor 1), espacio en blanco si está muerta (valor 0)
            std::cout << (board[idx(i, j, cols)] ? ALIVE : DEAD);
        }
        std::cout << "\t Proceso" << pid << "\n"; // Nueva línea al final de cada fila
    }
}

// ============================================================================
// FUNCIÓN PARA CONTAR VECINOS VIVOS DE UNA CÉLULA
// ============================================================================
int countLiveNeighbors(const std::vector<int>& board, int x, int y, int rows, int cols) {
    int count = 0;
    std::vector<int> temporal_M(9, 0);
    if(x ==0){
        if (y==0){
            temporal_M[idx(0,0,3)] = board[idx(rows-1,cols-1,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(rows-1,0,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(rows-1,1,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(0,cols-1,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(0,0,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(0,1,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(1,cols-1,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(1,0,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(1,1,cols)]; 
        } else if (y==cols-1){
            temporal_M[idx(0,0,3)] = board[idx(rows-1,cols-2,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(rows-1,cols-1,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(rows-1,0,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(0,cols-2,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(0,cols-1,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(0,0,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(1,cols-2,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(1,cols-1,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(1,0,cols)];
        } else{
            temporal_M[idx(0,0,3)] = board[idx(rows-1,y-1,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(rows-1,y,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(rows-1,y+1,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(0,y-1,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(0,y,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(0,y+1,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(1,y-1,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(1,y,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(1,y+1,cols)];
        } 
    } else if (y==0){
        if (x==rows-1){
            temporal_M[idx(0,0,3)] = board[idx(rows-2,cols-1,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(rows-2,0,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(rows-2,1,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(rows-1,cols-1,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(rows-1,0,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(rows-1,1,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(0,cols-1,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(0,0,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(0,1,cols)];
        } else {
            temporal_M[idx(0,0,3)] = board[idx(x-1,cols-1,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(x-1,0,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(x-1,1,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(x,cols-1,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(x,0,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(x,1,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(x+1,cols-1,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(x+1,0,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(x+1,1,cols)];
        } 
    } else if (x==rows-1){
        if(y==cols-1){
            temporal_M[idx(0,0,3)] = board[idx(rows-2,cols-2,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(rows-2,cols-1,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(rows-2,0,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(rows-1,cols-2,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(rows-1,cols-1,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(rows-1,0,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(0,cols-2,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(0,cols-1,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(0,0,cols)];
        } else {
            temporal_M[idx(0,0,3)] = board[idx(rows-2,y-1,cols)]; 
            temporal_M[idx(0,1,3)] = board[idx(rows-2,y,cols)]; 
            temporal_M[idx(0,2,3)] = board[idx(rows-2,y+1,cols)]; 
            temporal_M[idx(1,0,3)] = board[idx(rows-1,y-1,cols)]; 
            temporal_M[idx(1,1,3)] = board[idx(rows-1,y,cols)]; 
            temporal_M[idx(1,2,3)] = board[idx(rows-1,y+1,cols)]; 
            temporal_M[idx(2,0,3)] = board[idx(0,y-1,cols)]; 
            temporal_M[idx(2,1,3)] = board[idx(0,y,cols)]; 
            temporal_M[idx(2,2,3)] = board[idx(0,y+1,cols)];
        } 
    } else if (y==cols-1) {
        temporal_M[idx(0,0,3)] = board[idx(x-1,cols-2,cols)]; 
        temporal_M[idx(0,1,3)] = board[idx(x-1,cols-1,cols)]; 
        temporal_M[idx(0,2,3)] = board[idx(x-1,0,cols)]; 
        temporal_M[idx(1,0,3)] = board[idx(x,cols-2,cols)]; 
        temporal_M[idx(1,1,3)] = board[idx(x,cols-1,cols)]; 
        temporal_M[idx(1,2,3)] = board[idx(x,0,cols)]; 
        temporal_M[idx(2,0,3)] = board[idx(x+1,cols-2,cols)]; 
        temporal_M[idx(2,1,3)] = board[idx(x+1,cols-1,cols)]; 
        temporal_M[idx(2,2,3)] = board[idx(x+1,0,cols)];
    } else {
        temporal_M[idx(0,0,3)] = board[idx(x-1,y-1,cols)]; 
        temporal_M[idx(0,1,3)] = board[idx(x-1,y,cols)]; 
        temporal_M[idx(0,2,3)] = board[idx(x-1,y+1,cols)]; 
        temporal_M[idx(1,0,3)] = board[idx(x,y-1,cols)]; 
        temporal_M[idx(1,1,3)] = board[idx(x,y,cols)]; 
        temporal_M[idx(1,2,3)] = board[idx(x,y+1,cols)]; 
        temporal_M[idx(2,0,3)] = board[idx(x+1,y-1,cols)]; 
        temporal_M[idx(2,1,3)] = board[idx(x+1,y,cols)]; 
        temporal_M[idx(2,2,3)] = board[idx(x+1,y+1,cols)];
    } 
    
    // Por cada x,y, se crea una 'matriz' 3x3 donde x,y es el elemento 1,1 de esta nueva 'matriz'
    //
    //
    for (int ii = 0; ii <= 2; ii++)
        for (int jj = 0; jj <= 2; jj++)
            if (!(ii == 1 && jj == 1)) {
                count += temporal_M[idx(ii, jj, 3)];
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
// FUNCIÓN PARA INICIALIZAR EL TABLERO CON PATRÓN DETERMINISTA
// ============================================================================
std::vector<int> initializeBoard(const int &rows, const int &cols, const std::vector<int> &rows_per_proc, const int &nprocs, const int &pid, const int &non_int_div) {
    int local_rows = rows_per_proc[pid];
    std::mt19937 gen(SEED);
    std::uniform_real_distribution<> disunif(0.0, 1.0);
    int total_cells_before;
    if(non_int_div==0){
        total_cells_before = pid*rows_per_proc[0]*cols;
    } else {
        if (pid<non_int_div){
            total_cells_before = pid*(rows_per_proc[0])*cols;
        } else{
            total_cells_before = (non_int_div*(rows_per_proc[0])*cols)+(pid-non_int_div)*(rows_per_proc[0]-1)*cols;
        }
    }
    // Here, each process know how elements it has above
    for (int i = 0; i < total_cells_before; i++) disunif(gen);

    std::vector<int> local_board((local_rows)*cols, 0); // Two more rows are added in each local segment, because those will be filled just by the 
                                                          // the communication of MPI. One above and one below
    
    for (int i = 0; i < local_rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            local_board[idx(i, j, cols)] = (disunif(gen) < 0.4) ? 1 : 0;  // 40% probabilidad
        }
    }

    return local_board;
}

// ============================================================================
// FUNCIÓN PRINCIPAL - SIMULACIÓN DEL JUEGO
// ============================================================================
int main(int argc, char* argv[]) {
    // Inicializa el tablero con valores aleatorios reproducibles
    int rows = 10, cols = 10, generations = 10;
    bool print = false;
    std::string savefile = "";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--print") print = true;
        else if (arg == "--save" && i + 1 < argc) { savefile = argv[++i]; }
        else if (i + 2 < argc) {
            rows = std::stoi(argv[i]);
            cols = std::stoi(argv[i+1]);
            generations = std::stoi(argv[i+2]);
            i += 2;
        }
    }

    // Medición de tiempo usando std::chrono (sin dependencia de OpenMP)
    auto t0 = std::chrono::high_resolution_clock::now();
    MPI_Init(&argc, &argv);
    int nprocs, pid;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    //Suppose you have 100 lines of rows, and you want to divide evenly into 16 threads. How many rows should each thread process?
    int int_div = rows/nprocs; // This would be 6
    int non_int_div = rows%nprocs; // Note that this number is always less than nprocs by definition. This would be 4
    std::vector<int> rows_per_proc(nprocs, 0); // Vector to store how many rows each thread should process
    if (non_int_div == 0) {
        // this means that all threads must go even.
        for (int x = 0; x < nprocs; ++x) {
            rows_per_proc[x] = int_div; // each thread will process even
        }

    } else {
        for (int x = 0; x < nprocs; ++x) { // This would go from 0 to 15
            if (x < non_int_div) {
                rows_per_proc[x] = int_div + 1; // these threads will process one more row. This would be 4 threads that process 7 rows each, labbelled 0 to 3
            } else {
                rows_per_proc[x] = int_div; // these threads will process even. This would be 12 threads that process 6 rows each, labbelled 4 to 15
            }
        }
    }
    // Now you have a vector with the correct number of rows each thread should process.
    
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    auto board = initializeBoard(rows, cols, rows_per_proc, nprocs, pid, non_int_div);
    
    
    
    // Bucle principal: simula cada generación
    /*for (int gen = 0; gen < generations; ++gen) {
        // Muestra el número de la generación actual
        if (print) {
            //std::cout << "Generación: " << gen << "\n";
            printBoard(board, rows, cols);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Calcula y actualiza el tablero para la siguiente generación
        board = nextGeneration(board, rows, cols);
    }*/

    for (int gen = 0; gen < generations; ++gen) {
        // Muestra el número de la generación actual
        if (print) {
            //std::cout << "Generación: " << gen << "\n";
            printBoard(board, rows, cols, rows_per_proc, pid);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }



    MPI_Finalize();
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