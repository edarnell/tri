import cv2
import json

# Usage:
N_I = 30  # number of images
i_p = [f'frames/swim{i:02d}.png' for i in range(1, N_I+1)] # image paths
swim = cv2.imread(i_p[0])

class IA:  # Image Annotator
    def __init__(ia):  # ip: image paths, ap: annotation paths
        ia.CS = {
        'y': (0, 255, 255),  # Yellow
        'g': (0, 128, 0),    # Green
        'o': (0, 165, 255),  # Orange
        'p': (147, 20, 255), # Pink
        'w': (255, 255, 255) # White
        }
        ia.i = 0  # current image index
        ia.p = None  # current position
        try:
            with open('swim.json', 'r') as f:
                ia.an = json.load(f)
        except FileNotFoundError:
            ia.an = [[] for _ in range(N_I)]
        ia.annot()
        cv2.setMouseCallback('swim', ia.ev)
        ia.af()
    
    def ev(ia, e, x, y, f, p):  # e: event, x: x-coordinate, y: y-coordinate, f: flags, p: parameters
        if e == cv2.EVENT_LBUTTONDOWN:
            for i, s in enumerate(ia.an[ia.i]):
                for c, p in s.items():
                    px, py = p  # Extract the x and y coordinates of the point
                    distance = ((px - x) ** 2 + (py - y) ** 2) ** 0.5  # Calculate the Euclidean distance
                    if distance < 10:
                        del ia.an[ia.i][i]  # Remove the point if it's close to the clicked point
                        break
            ia.p = (x, y)
            ia.annot()

    def af(ia):  # key press
        while True:
            k = cv2.waitKey(0)
            c=chr(k & 0xFF)
            if c == '-':  # Left arrow key
                ia.i = max(0, ia.i - 1)  # Ensure index doesn't go below 0
            elif c == '+':  # Right arrow key
                ia.i = min(N_I - 1, ia.i + 1)  # Ensure index doesn't exceed the number of images
            elif k == 27:  # Escape key
                ia.an[ia.i] = []
            elif k == -1:
                with open('swim.json', 'w') as f:
                    json.dump(ia.an, f)
                print("Annotations saved.")
                # cv2.destroyAllWindows()
                break
            elif c in ia.CS:
                ia.an[ia.i].append({c: ia.p})
            else:
                print(f"Key '{c}' ignored.")
            ia.annot()

    def annot(ia):
        global swim
        swim = cv2.imread(i_p[ia.i])
        a = ia.an[ia.i]
        for an in a:
            for c,p in an.items():
                x, y = p
                cv2.circle(swim, (x, y), 8, ia.CS[c], -1)
        if (ia.p) is not None:
            cv2.circle(swim, ia.p, 4, (255, 0, 0), -1)
        cv2.imshow('swim', swim)
        cv2.setWindowTitle('swim', f'swim{ia.i+1:02d}')

IA()  # ia: Image Annotator instance