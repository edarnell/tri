import matplotlib.pyplot as plt
import matplotlib.patches as patches
from itertools import product

# Define colors and patterns
colors = ["blue", "yellow", "orange", "red", "cyan"]
patterns = ["square", "triangle", "circle", "diagonal", "vertical"]  # Added "vertical" pattern

def create_pattern(ax, bg_color, pattern_color, pattern):
    ax.set_xlim(0, 30)  # Adjusted for 1:3 width:height ratio
    ax.set_ylim(0, 10)
    ax.set_aspect('equal')
    ax.axis('off')
    
    # Background
    bg_rect = patches.Rectangle((0, 0), 30, 10, color=bg_color)  # Adjusted for 1:3 width:height ratio
    ax.add_patch(bg_rect)
    
    # Adjust pattern drawing code to fit the new subplot dimensions
    if pattern == "square":
        for x in range(2, 29, 10):  # Space for 3 squares
            square = patches.Rectangle((x, 2), 6, 6, color=pattern_color)  # Adjust size to be visible
            ax.add_patch(square)
    elif pattern == "triangle":
        for x in range(2, 29, 10):  # Space for 3 triangles
            # Lower the triangles slightly by adjusting the y-coordinates
            triangle = patches.Polygon([[x, 2], [x+6, 2], [x+3, 8]], color=pattern_color)  # Adjust size
            ax.add_patch(triangle)
    elif pattern == "circle":
        for x in range(5, 30, 10):  # Space for 3 circles
            circle = patches.Circle((x, 5), 3, color=pattern_color)  # Adjust radius to be visible
            ax.add_patch(circle)
    elif pattern == "diagonal":
        # 3 thick diagonal lines, make them wider
        dx=2
        for x in range(0, 31, 10):  # Adjust spacing for 3 lines
            ax.plot([x+dx, x+9], [0, 10], color=pattern_color, linewidth=20)  # Increase line thickness
    elif pattern == "vertical":
        # 3 vertical lines
        for x in range(7, 24, 8):  # Space for 3 vertical lines
            ax.plot([x, x], [0, 10], color=pattern_color, linewidth=20)  # Draw vertical lines

# Generate all combinations of background and pattern colors, excluding same color combinations, and patterns
color_combinations = [(bg, pat) for bg, pat in product(colors, colors) if bg != pat]
pattern_combinations = list(product(color_combinations, patterns))

# Ensure we only plot the first 100 combinations
pattern_combinations = pattern_combinations[:100]

# Create a figure to hold all subplots
fig, axs = plt.subplots(10, 10, figsize=(30, 10))  # Adjusted for 100 patterns with 1:3 width:height ratio

# Flatten the array of axes for easy iteration
axs_flat = axs.flatten()

# Iterate over combinations and plot each design
for i, ((bg_color, pattern_color), pattern) in enumerate(pattern_combinations):
    if i < len(axs_flat):  # Check to avoid IndexError
        create_pattern(axs_flat[i], bg_color, pattern_color, pattern)

plt.tight_layout()
plt.savefig("band.png")  # Save the figure as a PNG file
# plt.savefig("band.jpg", quality=95)  # Optionally save as a JPG file
# Optionally, set the DPI for the figure display as well
dpi_setting = 100  # Example DPI setting; adjust as needed for your specific requirements
fig.set_dpi(dpi_setting)
plt.show()




