import matplotlib.pyplot as plt
import numpy as np

# ==========================================
# ENTER YOUR DATA HERE
# ==========================================
# Replace the numbers inside the brackets with your actual execution times in seconds
# Order corresponds to threads: 1, 4, 16, 64, 256
execution_times = [1.65, 1.0, 0.6, 0.45, 0.45] 
# Note: The values above are just placeholders based on the image style. 
# PLEASE UPDATE THEM with your real results.

# X-axis values (Number of threads)
threads = [1, 4, 16, 64, 256]

# ==========================================
# PLOTTING CODE
# ==========================================

plt.figure(figsize=(8, 6))

# Plot the data
# color='#1db156' is a green similar to the reference image
# marker='o' adds the dots
plt.plot(threads, execution_times, marker='o', color='#1db156', linewidth=2, markersize=8)

# Set X-axis to logarithmic scale so the spacing between 1, 4, 16... is even
plt.xscale('log', base=4)

# Manually set the X-ticks to match the specific thread counts
plt.xticks(threads, labels=['1', '4', '16', '64', '256'], fontsize=14)
plt.yticks(fontsize=14)

# Add grid lines (horizontal only, dashed, light blue/gray)
plt.grid(axis='y', linestyle='--', alpha=0.7, color='#5b9bd5')

# Labels
plt.xlabel('# of threads', fontsize=16)
plt.ylabel('Execution time (s)', fontsize=16)

# Adjust y-axis limits slightly to make it look neat (optional)
# plt.ylim(0, max(execution_times) * 1.2)

# Remove the top and right spines (borders) to match the scientific style in the image
plt.gca().spines['top'].set_visible(False)
plt.gca().spines['right'].set_visible(False)

# Add arrow heads to axes (optional, to match the hand-drawn style exactly)
# This is a bit complex in standard matplotlib, so standard axes are usually preferred.

# Save the plot
plt.savefig('execution_time_plot.png', dpi=300)
plt.show()

print("Graph generated and saved as 'execution_time_plot.png'")