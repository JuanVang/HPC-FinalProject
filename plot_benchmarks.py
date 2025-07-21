import pandas as pd
import matplotlib.pyplot as plt

# Leer el CSV
df = pd.read_csv("benchmark_threads.csv")

# Filtrar solo la versión híbrida
hybrid = df[df['version'] == 'mpi_omp']

# Agrupar por número de hilos y calcular promedio
summary = hybrid.groupby('threads').agg({'time_sec':'mean', 'speedup':'mean', 'efficiency':'mean'}).reset_index()
procs = hybrid['procs'].iloc[0]

# Graficar Speed-up
plt.figure(figsize=(8,5))
plt.plot(summary['threads'], summary['speedup'], marker='o', label='Speed-up')
plt.plot(summary['threads'], summary['threads']*procs, 'k--', label='Ideal')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Speed-up')
plt.title(f'Speed-up vs. Número de hilos (MPI+OpenMP, np={procs})')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('speedup.png')
plt.show()

# Graficar Eficiencia
plt.figure(figsize=(8,5))
plt.plot(summary['threads'], summary['efficiency'], marker='o', color='orange', label='Eficiencia')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Eficiencia')
plt.title(f'Eficiencia vs. Número de hilos (MPI+OpenMP, np={procs})')
plt.grid(True)
plt.tight_layout()
plt.savefig('efficiency.png')
plt.show() 