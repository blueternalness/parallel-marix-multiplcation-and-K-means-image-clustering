import matplotlib.pyplot as plt
import numpy as np

execution_times = [28.047063, 10.323569, 8.819564, 8.590485, 8.434320] 

threads = [1, 4, 16, 64, 256]

plt.figure(figsize=(8, 6))

plt.plot(threads, execution_times, marker='o', color='#1db156', linewidth=2, markersize=8)

plt.xscale('log', base=4)

plt.xticks(threads, labels=['1', '4', '16', '64', '256'], fontsize=14)
plt.yticks(fontsize=14)

plt.grid(axis='y', linestyle='--', alpha=0.7, color='#5b9bd5')

plt.xlabel('# of threads', fontsize=16)
plt.ylabel('Execution time (s)', fontsize=16)

plt.gca().spines['top'].set_visible(False)
plt.gca().spines['right'].set_visible(False)

plt.savefig('execution_time_plot.png', dpi=300)
plt.show()

print("Graph generated and saved as 'execution_time_plot.png'")