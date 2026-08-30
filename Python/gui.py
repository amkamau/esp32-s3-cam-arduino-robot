import sys
import cv2
import threading
import time
import requests

from PySide6.QtCore import QTimer, Qt
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import (
    QApplication,
    QLabel,
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QGroupBox,
    QTextEdit,
    QLineEdit,
    QPushButton,
)


# ==========================================================
# ESP32 CONFIGURATION
# ==========================================================

ESP_IP = "192.168.0.50"

STREAM_URL = f"http://{ESP_IP}/api/camera/stream"
STATE_URL = f"http://{ESP_IP}/api/robot/state"
COMMAND_URL = f"http://{ESP_IP}/api/arduino/command"


# ==========================================================
# CAMERA SHARED DATA
# ==========================================================

latest_frame = None
frame_lock = threading.Lock()

camera_running = True


# ==========================================================
# STATE SHARED DATA
# ==========================================================

latest_state = ""
state_lock = threading.Lock()

state_running = True


# ==========================================================
# CAMERA THREAD
# ==========================================================

def camera_thread():

    global latest_frame
    global camera_running

    while camera_running:

        cap = None

        try:

            print("Connecting to camera...")

            cap = cv2.VideoCapture(
                STREAM_URL
            )

            if not cap.isOpened():

                print(
                    "Could not open camera stream"
                )

                time.sleep(2)
                continue

            print("Camera connected")

            while camera_running:

                ret, frame = cap.read()

                if not ret:

                    print(
                        "Camera stream disconnected"
                    )

                    break

                # Always keep ONLY the newest frame.
                with frame_lock:

                    latest_frame = frame

        except Exception as e:

            print(
                f"Camera error: {e}"
            )

        finally:

            if cap is not None:
                cap.release()

        if camera_running:

            time.sleep(2)

    print("Camera thread stopped")


# ==========================================================
# STATE THREAD
# ==========================================================

def state_thread():

    global latest_state
    global state_running

    session = requests.Session()

    while state_running:

        try:

            response = session.get(
                STATE_URL,
                timeout=1
            )

            if response.ok:

                with state_lock:

                    latest_state = response.text

            else:

                with state_lock:

                    latest_state = (
                        f"HTTP ERROR {response.status_code}"
                    )

        except requests.RequestException:

            with state_lock:

                latest_state = (
                    "Disconnected"
                )

        time.sleep(0.2)

    session.close()

    print("State thread stopped")


# ==========================================================
# MAIN WINDOW
# ==========================================================

class MainWindow(QMainWindow):

    def __init__(self):

        super().__init__()

        self.setWindowTitle(
            "ESP32 Robot Controller"
        )

        self.resize(
            1200,
            750
        )

        self.setup_ui()

        # --------------------------------------------------
        # CAMERA THREAD
        # --------------------------------------------------

        self.camera = threading.Thread(
            target=camera_thread,
            daemon=True
        )

        self.camera.start()

        # --------------------------------------------------
        # STATE THREAD
        # --------------------------------------------------

        self.state = threading.Thread(
            target=state_thread,
            daemon=True
        )

        self.state.start()

        # --------------------------------------------------
        # GUI VIDEO UPDATE
        # --------------------------------------------------

        self.video_timer = QTimer(self)

        self.video_timer.timeout.connect(
            self.update_video
        )

        self.video_timer.start(30)

        # --------------------------------------------------
        # GUI STATE UPDATE
        # --------------------------------------------------

        self.state_timer = QTimer(self)

        self.state_timer.timeout.connect(
            self.update_state
        )

        self.state_timer.start(100)

    # ======================================================
    # UI
    # ======================================================

    def setup_ui(self):

        # --------------------------------------------------
        # VIDEO
        # --------------------------------------------------

        self.image_label = QLabel(
            "Waiting for camera..."
        )

        self.image_label.setAlignment(
            Qt.AlignCenter
        )

        self.image_label.setMinimumSize(
            640,
            480
        )

        self.image_label.setStyleSheet(
            """
            QLabel {
                background-color: black;
                color: white;
                border: 1px solid #444;
            }
            """
        )

        # --------------------------------------------------
        # CONNECTION STATUS
        # --------------------------------------------------

        self.connection_label = QLabel(
            "ESP32: Connecting..."
        )

        self.connection_label.setAlignment(
            Qt.AlignCenter
        )

        self.connection_label.setStyleSheet(
            """
            QLabel {
                padding: 8px;
                font-weight: bold;
            }
            """
        )

        # --------------------------------------------------
        # STATE
        # --------------------------------------------------

        self.status_text = QTextEdit()

        self.status_text.setReadOnly(
            True
        )

        state_group = QGroupBox(
            "Robot State"
        )

        state_layout = QVBoxLayout()

        state_layout.addWidget(
            self.status_text
        )

        state_group.setLayout(
            state_layout
        )

        # --------------------------------------------------
        # COMMAND INPUT
        # --------------------------------------------------

        self.command_input = QLineEdit()

        self.command_input.setText(
            "1111001211501150"
        )

        self.command_input.setPlaceholderText(
            "Enter command..."
        )

        self.command_input.returnPressed.connect(
            self.send_command
        )

        # --------------------------------------------------
        # SEND BUTTON
        # --------------------------------------------------

        self.send_button = QPushButton(
            "SEND"
        )

        self.send_button.clicked.connect(
            self.send_command
        )

        # --------------------------------------------------
        # COMMAND GROUP
        # --------------------------------------------------

        command_group = QGroupBox(
            "Send Command"
        )

        command_layout = QVBoxLayout()

        command_layout.addWidget(
            self.command_input
        )

        command_layout.addWidget(
            self.send_button
        )

        command_group.setLayout(
            command_layout
        )

        # --------------------------------------------------
        # RESPONSE
        # --------------------------------------------------

        self.response_text = QTextEdit()

        self.response_text.setReadOnly(
            True
        )

        response_group = QGroupBox(
            "ESP Response"
        )

        response_layout = QVBoxLayout()

        response_layout.addWidget(
            self.response_text
        )

        response_group.setLayout(
            response_layout
        )

        # --------------------------------------------------
        # COMMUNICATION LOG
        # --------------------------------------------------

        self.log_text = QTextEdit()

        self.log_text.setReadOnly(
            True
        )

        log_group = QGroupBox(
            "Communication Log"
        )

        log_layout = QVBoxLayout()

        log_layout.addWidget(
            self.log_text
        )

        log_group.setLayout(
            log_layout
        )

        # --------------------------------------------------
        # RIGHT SIDE
        # --------------------------------------------------

        right_layout = QVBoxLayout()

        right_layout.addWidget(
            self.connection_label
        )

        right_layout.addWidget(
            state_group,
            2
        )

        right_layout.addWidget(
            command_group
        )

        right_layout.addWidget(
            response_group,
            1
        )

        right_layout.addWidget(
            log_group,
            2
        )

        # --------------------------------------------------
        # LEFT SIDE
        # --------------------------------------------------

        left_layout = QVBoxLayout()

        left_layout.addWidget(
            self.image_label,
            1
        )

        # --------------------------------------------------
        # MAIN LAYOUT
        # --------------------------------------------------

        main_layout = QHBoxLayout()

        main_layout.addLayout(
            left_layout,
            3
        )

        main_layout.addLayout(
            right_layout,
            1
        )

        central_widget = QWidget()

        central_widget.setLayout(
            main_layout
        )

        self.setCentralWidget(
            central_widget
        )

    # ======================================================
    # VIDEO UPDATE
    # ======================================================

    def update_video(self):

        global latest_frame

        with frame_lock:

            if latest_frame is None:
                return

            frame = latest_frame.copy()

        # BGR → RGB

        frame = cv2.cvtColor(
            frame,
            cv2.COLOR_BGR2RGB
        )

        height, width, channels = (
            frame.shape
        )

        image = QImage(
            frame.data,
            width,
            height,
            frame.strides[0],
            QImage.Format_RGB888
        )

        pixmap = QPixmap.fromImage(
            image
        )

        pixmap = pixmap.scaled(
            self.image_label.size(),
            Qt.KeepAspectRatio,
            Qt.SmoothTransformation
        )

        self.image_label.setPixmap(
            pixmap
        )

    # ======================================================
    # STATE UPDATE
    # ======================================================

    def update_state(self):

        global latest_state

        with state_lock:

            state = latest_state

        if state:

            self.status_text.setPlainText(
                state
            )

            if state != "Disconnected":

                self.connection_label.setText(
                    "ESP32: CONNECTED"
                )

                self.connection_label.setStyleSheet(
                    """
                    QLabel {
                        background-color: #225522;
                        color: white;
                        padding: 8px;
                        font-weight: bold;
                    }
                    """
                )

            else:

                self.connection_label.setText(
                    "ESP32: DISCONNECTED"
                )

                self.connection_label.setStyleSheet(
                    """
                    QLabel {
                        background-color: #442222;
                        color: white;
                        padding: 8px;
                        font-weight: bold;
                    }
                    """
                )

    # ======================================================
    # SEND COMMAND
    # ======================================================

    def send_command(self):

        command = (
            self.command_input.text()
        )

        if not command:
            return

        # Disable button while request is running.
        self.send_button.setEnabled(
            False
        )

        try:

            response = requests.post(
                COMMAND_URL,
                data=command,
                headers={
                    "Content-Type": "text/plain"
                },
                timeout=2
            )

            self.response_text.setPlainText(
                response.text
            )

            self.log_text.append(
                f"> {command}\n"
                f"< {response.text}\n"
            )

        except requests.RequestException as e:

            self.response_text.setPlainText(
                f"ERROR: {e}"
            )

            self.log_text.append(
                f"> {command}\n"
                f"< ERROR: {e}\n"
            )

        finally:

            self.send_button.setEnabled(
                True
            )

    # ======================================================
    # CLOSE
    # ======================================================

    def closeEvent(self, event):

        global camera_running
        global state_running

        camera_running = False
        state_running = False

        self.video_timer.stop()
        self.state_timer.stop()

        event.accept()


# ==========================================================
# APPLICATION
# ==========================================================

if __name__ == "__main__":

    app = QApplication(sys.argv)

    window = MainWindow()

    window.show()

    sys.exit(app.exec())