# ESP32-S3-CAM Arduino Robot

Robot control and camera system using an **ESP32-S3-CAM** and **Arduino**.

## Architecture

```text
                    ┌─────────────────────┐
                    │     PC / Client     │
                    │                     │
                    │  Python + PySide6   │
                    │  OpenCV             │
                    └──────────┬──────────┘
                               │
                         Wi-Fi / HTTP
                               │
                               ▼
                    ┌─────────────────────┐
                    │    ESP32-S3-CAM     │
                    │                     │
                    │  Camera             │
                    │  Web Server         │
                    │  Robot API          │
                    └──────────┬──────────┘
                               │
                              UART
                               │
                               ▼
                    ┌─────────────────────┐
                    │       Arduino       │
                    │                     │
                    │  Robot Control      │
                    │  Sensors            │
                    │  Servos             │
                    │  Motors             │
                    └─────────────────────┘
```

## Components

* **ESP32-S3-CAM** — camera streaming, Wi-Fi communication and HTTP API
* **Arduino** — real-time robot control
* **Camera** — video capture through ESP32-S3
* **Sensors** — robot/environment feedback
* **Motors & Servos** — robot movement and control
* **PC GUI** — video display and robot control

## Communication

```text
PC ──Wi-Fi/HTTP──> ESP32-S3-CAM ──UART──> Arduino
PC <─Wi-Fi/HTTP── ESP32-S3-CAM <─UART─── Arduino
```

The ESP32-S3-CAM acts as the communication bridge between the PC and Arduino.
