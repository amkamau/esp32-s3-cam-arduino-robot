## ESP32-S3 Camera App

A minimal live camera viewer that runs on a Freenove ESP32-S3-WROOM CAM. It serves a single-page web app over WiFi with a button to turn the OV3660 camera on and off and a full-frame MJPEG live feed.

---

### Hardware

| Item | Value |
|------|-------|
| Board | Freenove ESP32-S3-WROOM CAM (ESP32-S3-WROOM-1, N16R8) |
| Camera | OV3660 (3 MP) |
| Flash | 16 MB |
| PSRAM | 8 MB Octal |
| USB bridge | CH343 → `/dev/ttyUSB0` |

---

### Start the ESP-IDF Docker Environment

```bash
./start-docker.sh
```

Or manually:

```bash
docker run --rm -it --device=/dev/ttyUSB0 -v $(pwd):/project -w /project -u $(id -u) -e HOME=/tmp --group-add $(getent group uucp | cut -d: -f3) espressif/idf:v6.0.1
```

---

### Build, Flash and Monitor

```bash
idf.py set-target esp32s3   # only needed once, or when switching chips

idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Press `Ctrl+]` to exit the monitor.

---

### Project Structure

```
main/
├── main.c                    Boot logic — provisions or starts the web app
├── nvs_storage/              Read/write WiFi credentials to NVS flash
├── wifi/                     WiFi init, STA mode (connect), AP mode (hotspot)
├── http_server/              HTTP server wrapper (start, stop, register routes)
├── provisioning/             First-run WiFi setup page served over AP mode
├── camera/                   OV3660 init and MJPEG stream handler
└── web_app/
    ├── web_app.c             Route registration (/ and /api/camera/stream)
    └── index.html            Single-page camera viewer (embedded in firmware)
```

---

### Partition Table

| Name | Type | Offset | Size | Purpose |
|------|------|--------|------|---------|
| `nvs` | data/nvs | 0x9000 | 28 KB | WiFi credentials |
| `factory` | app/factory | 0x10000 | ~15.9 MB | Firmware and embedded web assets |

---

### First Run — WiFi Setup

On first boot the device has no WiFi credentials, so it starts in Access Point mode:

1. Connect your phone or laptop to the WiFi network **`ESP32-Setup`**
2. Open a browser and go to `192.168.4.1`
3. Enter your home WiFi SSID and password and click **Connect**
4. The device saves the credentials to NVS, reboots, and joins your network
5. Your device's IP address is printed to the serial monitor (e.g. `192.168.1.228`)

If the device fails to connect with stored credentials it falls back to provisioning mode automatically.

> Reflashing does **not** wipe NVS — your WiFi credentials survive a normal flash.
> To reset credentials, run `idf.py erase-flash` before flashing.

---

### Using the App

Open a browser and navigate to the device's IP address (shown in the serial monitor).

- Click **Turn on** to start the live camera feed (640×480 MJPEG, ~10 fps over WiFi)
- Click **Turn off** to stop the stream
