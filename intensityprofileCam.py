import cv2
import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import gaussian_filter1d

step_size_mm = 0.025  # mm per step (adjust if you're using microns)
num_avg_rows = 5    # Rows to average around image center
blur_sigma = 2 
camera_index = 0
cap = cv2.VideoCapture(camera_index, cv2.CAP_DSHOW)

if not cap.isOpened():
    print("Could not open camera.")
    exit()

cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1920)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1080) 

#let camera settle
for _ in range(8):
    ret, _ = cap.read()
    cv2.waitKey(40)

step_count = 0

input(f"Step {step_count+1}: Press Enter to capture...")

ret, frame = cap.read()
if not ret:
    print("failed to read frame.")
#perform zoom
"""h, w = frame.shape[:2]
center_x, center_y = w // 2, h // 2
zoom_fact = 25 #how much we zoom
half_w, half_h = w // (2 * zoom_fact), h // (2 * zoom_fact)

cropped = frame[center_y - half_h:center_y + half_h,
                center_x - half_w:center_x + half_w]

zoomed_frame = cv2.resize(cropped, (w, h), interpolation=cv2.INTER_LINEAR)"""

#take the slice
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
h, w = gray.shape

# === Take horizontal slice ===
center_y = h // 2
strip = gray[center_y - num_avg_rows//2:center_y + num_avg_rows//2 + 1, :]
profile = np.mean(strip, axis=0)



#getting midpoint of split
distance = np.arange(len(profile))

#display intensity profile
plt.figure(figsize=(6, 4))
plt.plot(distance, profile, marker='o')
plt.xlabel("Pixels")
plt.ylabel("Intensity")
plt.title("Intensity Profile of Stream")
plt.grid(True)
plt.tight_layout()
plt.show()

#Smooth and compute gradient
profile_smooth = gaussian_filter1d(profile, sigma=blur_sigma)
gradient = np.gradient(profile_smooth)
edge_x = np.argmax(gradient)

cv2.line(frame, (int(edge_x), 0), (int(edge_x), h), (0, 255, 0), 2)
cv2.imshow("Webcam: ", frame)

if cv2.waitKey(1) == ord('q'):
    cap.release()
    cv2.destroyAllWindows()