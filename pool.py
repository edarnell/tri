import cv2
import numpy as np

# List to store the points
points = []

def order_points(pts):
    # Only consider the first four points
    pts = pts[:4]

    # Sort the points based on their y-coordinates
    y_sorted = pts[np.argsort(pts[:, 0]), :]

    # Get the top-most and bottom-most points
    top_most = y_sorted[:2, :]
    bottom_most = y_sorted[2:, :]

    # Sort the top-most points based on their x-coordinates
    # The top-left point will have the smallest x-coordinate
    top_most = top_most[np.argsort(top_most[:, 1]), :]
    (tl, tr) = top_most

    # Calculate the Euclidean distance between the top-most points
    # The bottom-right point will have the largest distance
    D = np.linalg.norm(tr - bottom_most, axis=1)
    (br, bl) = bottom_most[np.argsort(D)[::-1], :]

    # Return the ordered coordinates
    return np.array([tl, tr, br, bl], dtype="float32")

# Mouse callback function
def select_point(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        # Draw a circle at the clicked point
        cv2.circle(frame, (x, y), 5, (0, 255, 0), -1)

        # Draw a line from the last point to the clicked point
        if points:
            cv2.line(frame, points[-1], (x, y), (0, 255, 0), 1)

        # Add the clicked point to the list of points
        points.append((x, y))

        # Redisplay the frame
        cv2.imshow("Frame", frame)

# Set the mouse callback function
cv2.namedWindow("Frame")
cv2.setMouseCallback("Frame", select_point)

# Open the video
cap = cv2.VideoCapture('swim2.mp4')
# Capture a frame
ret, frame = cap.read()

# Check if the frame was successfully captured
if not ret:
    print("Failed to capture frame")
    exit(1)

# Display the frame
cv2.imshow("Frame", frame)

# Wait until four points have been selected and the last point is near the first point
while len(points) < 4 or np.linalg.norm(np.array(points[0]) - np.array(points[-1])) > 20:
    cv2.waitKey(1)
# List to store the trackers
trackers = []

while True:
    # Capture a frame
    ret, frame = cap.read()

    # Check if the frame was successfully captured
    if not ret:
        break
    # Check if four or more points have been selected
    if len(points) >= 4:
        # Order the points
        ordered_points = order_points(np.array(points))
        # Define destination points
        dst_pts = np.float32([[0, 0], [frame.shape[1], 0], [0, frame.shape[0]], [frame.shape[1], frame.shape[0]]])

        # Compute the perspective transformation matrix
        M = cv2.getPerspectiveTransform(ordered_points, dst_pts)

        # Warp the frame
        frame = cv2.warpPerspective(frame, M, (frame.shape[1], frame.shape[0]))

        # Convert the frame to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Detect contours
        contours, _ = cv2.findContours(gray, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        # Iterate over the contours
        for contour in contours:
            # Compute the bounding box for the contour
            (x, y, w, h) = cv2.boundingRect(contour)

            # Extract the sub-frame corresponding to the bounding box
            sub_frame = frame[y:y+h, x:x+w]

            # Check if the sub-frame is not empty
            if sub_frame.size > 0:
                # Initialize a tracker for the bounding box
                tracker = cv2.TrackerKCF_create()
                success = tracker.init(frame, (x, y, w, h))

                # Add the tracker to the list of trackers
                if success:
                    trackers.append(tracker)

        # Display the number of contours and the size of the sub-frames
        cv2.putText(frame, f"Contours: {len(contours)}", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        cv2.putText(frame, f"Sub-frame size: {sub_frame.size if 'sub_frame' in locals() else 0}", (10, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)    # Update the trackers
    for tracker in trackers:
        # Update the tracker
        success, box = tracker.update(frame)

        # If the tracking was successful, draw the bounding box
        if success:
            (x, y, w, h) = [int(v) for v in box]
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    # Display the number of trackers and the success of the tracking
    # cv2.putText(frame, f"Trackers: {len(trackers)}", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
    # cv2.putText(frame, f"Success: {success}", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    # Display the frame
    cv2.imshow("Frame", frame)

    # Break the loop if 'q' is pressed
    if cv2.waitKey(25) & 0xFF == ord('q'):
        break

# Release the video capture and close the windows
cap.release()
cv2.destroyAllWindows()