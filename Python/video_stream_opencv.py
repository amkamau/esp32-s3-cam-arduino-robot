import cv2

ESP_IP = "192.168.0.50"
STREAM_URL = f"http://{ESP_IP}/api/camera/stream"

cap = cv2.VideoCapture(STREAM_URL)

if not cap.isOpened():
    print("Failed to open stream")
    exit()

print("Stream connected")

while True:

    ret, frame = cap.read()

    if not ret:
        print("Failed to read frame")
        break

    cv2.imshow("ESP32 Camera", frame)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()