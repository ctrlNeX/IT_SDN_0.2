import math
import random
import sys

JUST_PRINT = False
# JUST_PRINT = True

n = 16
down_prob = 0.1
links = []

sqrtn = int (math.sqrt(n))

if not JUST_PRINT:
    f = file("links.dat",'w')
    SOME_TO_ALL = False
    some_nodes = [0]
    RANDOM_LINK_DOWN = False

    f.write("#SRC DST PRR . . . RSSI . .\n")
    for src in range(n):
        line = src / sqrtn
        line_members = range(line * sqrtn, (line + 1) * sqrtn)
        print line, sqrtn
        print line_members

        neighbors = []
        if SOME_TO_ALL and src in some_nodes:
            neighbors = range(n)
            neighbors.remove(src)
        else:
            if src + 1 in line_members:
                neighbors += [src + 1]
            if src - 1 in line_members:
                neighbors += [src - 1]
            if src + sqrtn in range(n):
                neighbors += [src + sqrtn]
            if src - sqrtn in range(n):
                neighbors += [src - sqrtn]

        if RANDOM_LINK_DOWN:
            print "sim"
            for dst in neighbors:
                if random.random() <= down_prob:
                    neighbors.remove(dst)

        for dst in neighbors:
            links += [(src, dst)]
            line = "%d %d 1 ? ? ? 0 ? ?" % (src+1, dst+1)
            f.write(line + '\n')
    f.close()
else: # JUST_PRINT = true
    f = file("links.dat",'r')
    for line in f:
        print line
        if not line.startswith("#"):
            spl = line.strip().split()
            links += [(int(spl[0])-1,int(spl[1])-1)]
    f.close()

for l in links:
    print l

for line in range(sqrtn):
    for column in range(sqrtn):
        current_node = line + column*sqrtn
        sys.stdout.write("%2d " % (current_node + 1,))
        if column == range(sqrtn)[-1]:
             pass
        else:
            if (current_node+sqrtn, current_node) in links:
                sys.stdout.write("<")
                if (current_node, current_node+sqrtn) in links:
                    sys.stdout.write("->")
                else:
                    sys.stdout.write("--")
            else:
                if (current_node, current_node+sqrtn) in links:
                    sys.stdout.write("-->")
                else:
                    sys.stdout.write("   ")
    print

    if line != range(sqrtn)[-1]:
        for column in range(sqrtn):
            current_node = line + column*sqrtn
            if (current_node+1, current_node) in links:
                sys.stdout.write(" ^")
            else:
                if (current_node, current_node+1) in links:
                    sys.stdout.write(" |")
                else:
                    sys.stdout.write("  ")
            sys.stdout.write("    ")
        print

        for column in range(sqrtn):
            current_node = line + column*sqrtn
            if (current_node+1, current_node) in links or (current_node, current_node+1) in links:
                sys.stdout.write(" |")
            else:
                sys.stdout.write("  ")
            sys.stdout.write("    ")
        print

        for column in range(sqrtn):
            current_node = line + column*sqrtn
            if (current_node, current_node+1) in links:
                sys.stdout.write(" v")
            else:
                if (current_node+1, current_node) in links:
                    sys.stdout.write(" |")
                else:
                    sys.stdout.write("  ")
            sys.stdout.write("    ")
        print

    # print line

print "Missing links: ", "%.2f" % (100 - 100 * (len(links) / (2*(4.0*n/2 - 2*sqrtn))),)
