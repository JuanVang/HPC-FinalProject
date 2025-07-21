import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("benchmark_threads.csv")

# OpenMP: total_threads = threads
omp = df[df['version'] == 'omp'].copy()
omp['total_threads'] = omp['threads']
omp_summary = omp.groupby('total_threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()

# MPI+OpenMP: solo combinaciones balanceadas
mpi_omp = df[df['version'] == 'mpi_omp'].copy()
balanceadas = [(1,16), (2,8), (4,4), (8,2)]
mpi_omp_bal = mpi_omp[mpi_omp.apply(lambda row: (row['procs'], row['threads']) in balanceadas, axis=1)]
mpi_omp_bal['label'] = mpi_omp_bal['procs'].astype(int).astype(str) + 'p-' + mpi_omp_bal['threads'].astype(int).astype(str) + 'h'
mpi_omp_summary = mpi_omp_bal.groupby(['procs','threads','label']).agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()

# Graficar Speed-up
plt.figure(figsize=(8,5))
plt.plot(omp_summary['total_threads'], omp_summary['speedup'], marker='o', label='OpenMP')
plt.plot(mpi_omp_summary['procs'], mpi_omp_summary['speedup'], marker='s', label='MPI+OpenMP (16 hilos totales)')
for _, row in mpi_omp_summary.iterrows():
    plt.annotate(row['label'], (row['procs'], row['speedup']), textcoords="offset points", xytext=(0,10), ha='center')
plt.xlabel('Procesos MPI (MPI+OpenMP) / Hilos (OpenMP)')
plt.ylabel('Speed-up')
plt.title('Speed-up: OpenMP vs. MPI+OpenMP (16 hilos totales)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('speedup_vs_procs.png')
plt.show()

# Graficar Eficiencia
plt.figure(figsize=(8,5))
plt.plot(omp_summary['total_threads'], omp_summary['efficiency'], marker='o', label='OpenMP')
plt.plot(mpi_omp_summary['procs'], mpi_omp_summary['efficiency'], marker='s', label='MPI+OpenMP (16 hilos totales)')
for _, row in mpi_omp_summary.iterrows():
    plt.annotate(row['label'], (row['procs'], row['efficiency']), textcoords="offset points", xytext=(0,10), ha='center')
plt.xlabel('Procesos MPI (MPI+OpenMP) / Hilos (OpenMP)')
plt.ylabel('Eficiencia')
plt.title('Eficiencia: OpenMP vs. MPI+OpenMP (16 hilos totales)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('efficiency_vs_procs.png')
plt.show() 