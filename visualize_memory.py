import matplotlib.pyplot as plt
import matplotlib.patches as patches

def parse_memory_log(filename="memory_log.txt"):
    operations = []
    with open(filename, "r") as file:
        for line in file:
            parts = line.split()
            operation = parts[0]
            address = int(parts[1], 16)
            size = int(parts[2])
            operations.append((operation, address, size))
    return operations

def visualize_memory_operations(operations, image_filename="memory_visualization.png"):
    fig, ax = plt.subplots(figsize=(8, 6))
    current_y = 0
    memory_blocks = {}

    for op, addr, size in operations:
        if op == "ALLOCATE":
            # Add a new block
            memory_blocks[addr] = size
        elif op == "FREE" and addr in memory_blocks:
            # Remove a block
            del memory_blocks[addr]

    for addr, size in sorted(memory_blocks.items()):
        rect = patches.Rectangle((0, current_y), 5, size, linewidth=1,
                                 edgecolor='black', facecolor='skyblue')
        ax.add_patch(rect)
        ax.text(6, current_y + size / 2, f"Addr: {hex(addr)}\nSize: {size}", va="center", ha="left")
        current_y += size + 5  # Space between blocks

    ax.set_xlim(0, 10)
    ax.set_ylim(0, current_y + 10)
    ax.axis('off')

    plt.title("Heap Memory Visualization")
    plt.savefig(image_filename)
    plt.show()

if __name__ == "__main__":
    operations = parse_memory_log()
    visualize_memory_operations(operations)
