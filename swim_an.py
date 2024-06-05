import cv2
import json
import string

# Usage:
N_I = 31  # number of images
i_p = [f'frames/swim{i:02d}.png' for i in range(1, N_I)] # image paths

class IA:  # Image Annotator
    def __init__(ia):  # ip: image paths, ap: annotation paths
        ia.SM = {
        'w': 'waiting',
        'r': 'resting',
        's': 'swimming',
        'u': 'submerged'
        }
        ia.CS = {
        'y': (0, 255, 255),  # Yellow
        'g': (0, 128, 0),    # Green
        'o': (0, 165, 255),  # Orange
        'p': (147, 20, 255), # Pink
        'w': (255, 255, 255) # White
        }
        ia.ci = 0  # current image index
        ia.ti = ""  # current text
        ia.p = None  # current position
        try:
            with open('swim.json', 'r') as f:
                ia.an = json.load(f)
        except FileNotFoundError:
            ia.an = [[] for _ in range(N_I)]
        ia.annot()
        cv2.setMouseCallback('image', ia.ev)
        ia.af()
    
    def ev(ia, e, x, y, f, p):  # e: event, x: x-coordinate, y: y-coordinate, f: flags, p: parameters
        if e == cv2.EVENT_LBUTTONDOWN:
            ia.ti = ""
            ia.p = (x, y)
            ic = ia.im.copy()
            cv2.circle(ic, (x, y), 4, (0, 255, 0), -1)
            cv2.imshow('image', ic)

    def af(ia):  # key press
        while True:
            k = cv2.waitKey(0)
            print(k)
            if k == 8:  # Backspace key
                ia.ti = ia.ti[:-1]
            elif k == ord('-'):  # Left arrow key
                ia.ci = max(0, ia.ci - 1)  # Ensure index doesn't go below 0
            elif k == ord('+'):  # Right arrow key
                ia.ci = min(N_I - 1, ia.ci + 1)  # Ensure index doesn't exceed the number of images
            elif k == 27:  # Escape key
                ia.an[ia.ci] = []
            elif k == -1:
                with open('swim.json', 'w') as f:
                    json.dump(ia.an, f, indent=4)
                print("Annotations saved.")
                # cv2.destroyAllWindows()
                break
            elif chr(k) in string.printable:
                ia.ti += chr(k)
            ia.img()

    def annot(ia, i=None):
        ia.im = cv2.imread(i_p[ia.ci])
        a = ia.an[ia.ci] if i is None else [ia.an[ia.ci][i]]
        for an in a:
            x, y = an['p']
            c, n = an['c'], an['n']
            cv2.circle(ia.im, (x, y), 12, ia.CS[c], -1)
            (w, h), _ = cv2.getTextSize(f"{n}", cv2.FONT_HERSHEY_SIMPLEX, 0.5, 2)
            cv2.putText(ia.im, f"{n}", (x-w//2, y+h//2), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,0), 2)
        cv2.imshow('image', ia.im)

    def img(ia):
        if len(ia.ti) == 3:
            c, n, s = ia.ti[0], ia.ti[1], ia.ti[2]
            if ia.ci not in ia.an:
                ia.an[ia.ci] = []
            ia.an[ia.ci].append({"p": ia.p, "c": c, "s": s, "n": n})
            ia.annot(-1)
        elif ia.ti:
            ic = ia.im.copy()
            cv2.putText(ic, ia.ti, ia.p, cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            cv2.imshow('image', ic)

IA()  # ia: Image Annotator instance