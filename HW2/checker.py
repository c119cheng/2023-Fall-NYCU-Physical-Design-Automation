import matplotlib.pyplot as plt
import sys
import matplotlib.patches as patches

# usage : ./checher.py [input data file] [placement result] [output fig. name]
input_file = sys.argv[1]
place_file = sys.argv[2]
out_fig = sys.argv[3]

# read input data
r_low, r_upper = 0, 0
block_dic = {}
with open(input_file) as f:
    # read aspect ratio
    lines = f.readlines()
    print(lines[0])
    r_low, r_upper = float(lines[0].split()[0]), float(lines[0].split()[1])

    # read block info
    for line in lines[1:]:
        words = line.split()
        block_dic[words[0]] = [int(words[1]), int(words[2])]


# read placement result
area = 0
r = 0
place_dic = {}
rotated = {}
with open(place_file) as f:
    lines = f.readlines()
    area = int(lines[0].split()[2])
    r = float(lines[1].split()[2])

    for line in lines[2:]:
        words = line.split()
        place_dic[words[0]] = [int(w) for w in words[1:3]]
        if len(words) == 4:
            rotated[words[0]] = True
        else:
            rotated[words[0]] = False

# plot result and verify
fig, ax = plt.subplots()
max_x, max_y = 0, 0
for key in place_dic.keys():
    w, h, x, y = block_dic[key][0], block_dic[key][1], place_dic[key][0], place_dic[key][1]
    
    if rotated[key]:
        tmp = w
        w = h
        h = tmp

    max_x = max(max_x, x+w)
    max_y = max(max_y, y+h)
    rect = patches.Rectangle((x, y), w, h, linewidth=1, edgecolor='r', facecolor='b')
    ax.add_patch(rect)

    # add text
    # ax.text(int(x + w/2), int(y + h/2), key, fontsize=12, color='red')

c = max(max_x, max_y)
ax.set_xlim(0, c + 5)
ax.set_ylim(0, c + 5)

plt.savefig(out_fig)

print(f'golden area = {max_x * max_y}')
print(f'golden ratio = {max_x / max_y}')
print(f'output area = {area}')
print(f'output ratio = {r}')