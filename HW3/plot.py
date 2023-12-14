import matplotlib.pyplot as plt
import sys
import matplotlib.patches as patches
import random

def plot_label(ax, label, cx, cy, is_gate):
    if is_gate:
        ax.annotate(label, (cx, cy), color='w', ha='center', va='center')
    else:
        ax.annotate(label, (cx, cy), color='k', ha='center', va='center')

# usage : ./checher.py [input data file] [placement result] [output fig. name]
input_file = sys.argv[1]
place_file = sys.argv[2]
out_fig = sys.argv[3]

# read place result
with open(place_file, 'r') as f:
    lines = f.readlines()
    pmos_list = lines[1].split()
    pmos_pins = lines[2].split()
    nmos_list = lines[3].split()
    nmos_pins = lines[4].split()

# plot pins
fig, ax = plt.subplots()
fig.tight_layout()

cur_x = 0
diffusion_width = 34
diffusion_height = 20
nmos_y = 20
pmos_y = 60
gate_width = 20
gate_height = 100
gate_y = 0
prev_gate = True
for n_pin, p_pin in zip(nmos_pins, pmos_pins):
    if n_pin == "Dummy":
        poly1 = patches.Rectangle((cur_x, gate_y), gate_width, gate_height, edgecolor='b', facecolor='b')
        plot_label(ax, 'D', cur_x+gate_width/2, gate_y+gate_height/2, is_gate=True)
        cur_x += gate_width
        diffusion_n = patches.Rectangle((cur_x, nmos_y), diffusion_width, diffusion_height, edgecolor='g', facecolor='g')
        plot_label(ax, 'D', cur_x+diffusion_width/2, nmos_y+diffusion_height/2, is_gate=False)
        diffusion_p = patches.Rectangle((cur_x, pmos_y), diffusion_width, diffusion_height, edgecolor='g', facecolor='g')
        plot_label(ax, 'D', cur_x+diffusion_width/2, pmos_y+diffusion_height/2, is_gate=False)
        cur_x += diffusion_width
        poly2 = patches.Rectangle((cur_x, gate_y), gate_width, gate_height, edgecolor='b', facecolor='b')
        plot_label(ax, 'D', cur_x+gate_width/2, gate_y+gate_height/2, is_gate=True)
        cur_x += gate_width
        ax.add_patch(poly1)
        ax.add_patch(poly2)
        ax.add_patch(diffusion_n)
        ax.add_patch(diffusion_p)
        prev_gate = True
    else:
        if prev_gate == True:
            diffusion_n = patches.Rectangle((cur_x, nmos_y), diffusion_width, diffusion_height, edgecolor='g', facecolor='g')
            diffusion_p = patches.Rectangle((cur_x, pmos_y), diffusion_width, diffusion_height, edgecolor='g', facecolor='g')
            plot_label(ax, n_pin, cur_x+diffusion_width/2, nmos_y+diffusion_height/2, is_gate=False)
            plot_label(ax, p_pin, cur_x+diffusion_width/2, pmos_y+diffusion_height/2, is_gate=False)
            cur_x += diffusion_width
            ax.add_patch(diffusion_n)
            ax.add_patch(diffusion_p)
            prev_gate = False
        else:
            poly1 = patches.Rectangle((cur_x, gate_y), gate_width, gate_height, edgecolor='b', facecolor='b')
            plot_label(ax, n_pin, cur_x+gate_width/2, gate_y+gate_height/2, is_gate=True)
            cur_x += gate_width
            ax.add_patch(poly1)
            prev_gate = True

# plot wire
nets = {}
with open(input_file, 'r') as f:
    # read .sp
    lines = f.readlines()
    lines = lines[1:-1]
    
    for line in lines:
        # read nets
        mos = line.split()
        nets[mos[1]] = []
        nets[mos[3]] = []

cur_x = diffusion_width/2
prev_gate = True
for n_pin, p_pin in zip(nmos_pins, pmos_pins):
    if n_pin == "Dummy":
        cur_x += (gate_width + diffusion_width)
        prev_gate = True
    elif prev_gate:
        nets[n_pin].append([cur_x, nmos_y + diffusion_height/2])
        nets[p_pin].append([cur_x, pmos_y + diffusion_height/2])
        cur_x += gate_width + diffusion_width
        prev_gate = False
    else:
        prev_gate = True

cur_x += diffusion_width/2
for net in nets:
    min_x = min(pin[0] for pin in nets[net])
    max_x = max(pin[0] for pin in nets[net])
    min_y = min(pin[1] for pin in nets[net])
    max_y = max(pin[1] for pin in nets[net])
    
    for pin in nets[net]:
        ax.plot(pin[0], pin[1], 'ro')  # 'ro' stands for red color and circle marker

    # Generate a random coordinate shift in the range [-5, 5]
    shift_x = random.uniform(-2, 2)
    shift_y = random.uniform(-2, 2)

    # Apply the random shift to the coordinates
    min_x += shift_x
    max_x += shift_x
    min_y += shift_y
    max_y += shift_y


    random_color ="#{:02x}{:02x}{:02x}".format(
    random.randint(150, 255),
    random.randint(150, 255),
    random.randint(150, 255)
    )    # Generate a random hexadecimal color code
    ax.plot([min_x, max_x, max_x, min_x, min_x], [min_y, min_y, max_y, max_y, min_y], color=random_color, linestyle='-')


figsize = (cur_x+20)/80, (150+20)/80
fig.set_size_inches(figsize[0], figsize[1])
# fig.figsize(figsize)
ax.set_xlim(0, cur_x+5)
ax.set_ylim(0, 105)
plt.savefig(out_fig)