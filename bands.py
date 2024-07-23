import matplotlib.pyplot as plt
import numpy as np
from itertools import product

# Define colors and patterns
colors = ["blue", "yellow", "orange", "purple", "cyan"]
patterns = ["stripes", "dots", "squares", "diagonal stripes"]

# Function to create patterns with background and pattern color
def create_pattern(ax, bg_color, pattern_color, pattern):
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 1)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_facecolor(bg_color)
    if pattern == "stripes":
        for i in range(10):
            if i % 2 == 0:
                ax.fill_betweenx([0, 1], i, i+1, color=pattern_color)
    elif pattern == "dots":
        for i in range(5):
            for j in range(2):
                circle = plt.Circle((2*i+1, 0.5), 0.4, color=pattern_color)
                ax.add_patch(circle)
    elif pattern == "squares":
        for i in range(5):
            ax.fill_between([2*i, 2*i+1], 0, 1, color=pattern_color)
    elif pattern == "diagonal stripes":
        for i in range(10):
            if i % 2 == 0:
                ax.fill_between([i, i+1], 0, 1, color=pattern_color, hatch='/')

# Create list of color-pattern combinations ensuring no repeat of the same fg and bg color
combinations = [(bg_color, pattern_color, pattern) for bg_color in colors for pattern_color in colors if bg_color != pattern_color for pattern in patterns]

# Ensure we have exactly 100 unique combinations
combinations = combinations[:100]

# Generate wristband designs and save
fig, axs = plt.subplots(10, 10, figsize=(20, 20))
fig.subplots_adjust(hspace=0.5, wspace=0.5)

for idx, (bg_color, pattern_color, pattern) in enumerate(combinations):
    ax = axs[idx // 10, idx % 10]
    create_pattern(ax, bg_color, pattern_color, pattern)

# Save the figure
fig.savefig("bands.png", bbox_inches='tight')
plt.close(fig)




