import cv2
import json
import numpy as np
import string

an = [] # an: List to store annotations
ti = "" # ti: String to store current input
p = None # p: Tuple to store current input location

sm = {
    'w': 'waiting',
    'r': 'resting',
    's': 'swimming',
    'u': 'submerged'
}

# Mouse callback function
def ev(e, x, y, flags, param):
    global an, im, ti, p
    if e == cv2.EVENT_LBUTTONDOWN:
        p = (x, y)
        ti = ""

# Keyboard callback function
def ea(key):
    global an, p, ti
    if len(ti) == 3:
        c, n, s = ti[0], ti[1], ti[2]
        # c y=yellow, g=green, o=orange, p=pink, w=white
        colors = {
    'c': (0, 255, 255),  # Cyan
    'y': (0, 255, 255),  # Yellow
    'g': (0, 128, 0),    # Green
    'o': (0, 165, 255),  # Orange
    'p': (147, 20, 255), # Pink
    'w': (255, 255, 255) # White
}
        an.append({"p": p, "c": c, "s": s, "n": n})
        x, y = p
        cv2.circle(im, (x, y), 12, colors[c], -1)
        # Calculate the size of the text box
        (w, h), _ = cv2.getTextSize(f"{n}", cv2.FONT_HERSHEY_SIMPLEX, 0.5, 2)
        cv2.putText(im, f"{n}", (x-w//2, y+h//2), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,0), 2)
        ti = ""
        p = None
    elif key == 8:  # Backspace key
        ti = ti[:-1]
    else:
        ch = chr(key)
        if ch in string.printable:
            ti += ch

def af(ip, af):
    global im, an, ti, p
    an = []
    ti = ""
    p = None
    im = cv2.imread(ip)
    cv2.namedWindow('image')
    cv2.setMouseCallback('image', ev)
    while True:
        ic = im.copy()
        if p:
            cv2.putText(ic, ti, p, cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        cv2.imshow('image', ic)
        key = cv2.waitKey(1) & 0xFF
        if key == 27:  # Press 'ESC' to exit
            break
        elif p:
            ea(key)
    cv2.destroyAllWindows()
    with open(af, 'w') as f:
        json.dump(an, f, indent=4)
    print(f"Annotations saved to {af}")

for i in range(1, 31):
    ip = f'frames/swim{i:02d}.png'
    ans = f'an_swim{i:02d}.json'
    af(ip, ans)