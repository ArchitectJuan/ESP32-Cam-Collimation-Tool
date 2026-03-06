# ESP32-Cam Collimation Tool

This project provides a collimation tool using an ESP32-Cam. It hosts a web interface that can be used to stream the camera feed and overlay a customizable collimation reticle.

## 📥 How to Download the Files

### Option 1: Download as ZIP
1. Go to the [GitHub repository page](https://github.com/ArchitectJuan/ESP32-Cam-Collimation-Tool).
2. Click on the green **Code** button.
3. Select **Download ZIP**.
4. Extract the downloaded ZIP file to a folder on your computer. 
5. **Important:** Make sure to rename the extracted folder to `ESP32_Collimator` (remove the `-master` or `-main` suffix if present) so it matches the `.ino` file name exactly.

### Option 2: Clone with Git
If you have Git installed, you can clone the repository directly using your terminal or command prompt:
```bash
git clone https://github.com/ArchitectJuan/ESP32-Cam-Collimation-Tool.git
```

## ⚡ How to Flash with Arduino IDE

### Prerequisites
1. Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Add ESP32 board support to Arduino IDE:
   - Go to **File > Preferences**.
   - In "Additional Boards Manager URLs", add the following URL:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to **Tools > Board > Boards Manager**, search for `esp32`, and install it.

### Flashing the ESP32-CAM
1. Open the Arduino IDE.
2. Go to **File > Open** and select the `ESP32_Collimator.ino` file from the downloaded folder.
3. Connect your ESP32-CAM to your computer using an FTDI programmer (Make sure you connect `U0T` to `RX`, `U0R` to `TX`, and ground `GPIO 0` while powering up to put the board into flash mode).
4. Go to the **Tools** menu and configure the following settings:
   - **Board:** `AI Thinker ESP32-CAM`
   - **CPU Frequency:** `240MHz (WiFi/BT)`
   - **Flash Frequency:** `80MHz`
   - **Flash Mode:** `QIO`
   - **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)` — *Required for the camera's memory footprint.*
   - **Port:** Select the COM port your FTDI programmer is connected to.
5. Click the **Upload** arrow button in the Arduino IDE.
6. Once uploading is complete ("Leaving... Hard resetting via RTS pin..."), disconnect `GPIO 0` from GND and press the **Reset** button on the ESP32-CAM.

## 🌐 How to Log into the Web Interface

Once the ESP32-CAM is powered up and running, follow these steps to use the tool:

1. **Connect to the ESP32 Wi-Fi Network:**
   - Open the Wi-Fi settings on your phone, tablet, or computer.
   - Look for the network named **`ESP32_Collimator`**.
   - Connect to this network. (It is an open network, so no password is required by default).

2. **Access the Interface:**
   - Open a web browser (Chrome, Safari, Firefox, Edge, etc.).
   - Type `http://192.168.4.1` into the address bar and press **Enter**.
   - *Alternatively, you may try navigating to `http://collimator.local` if your device supports mDNS.*

3. **Using the Tool:**
   - The camera stream will appear in the center of the web page with a collimation reticle overlaid on it.
   - You can use the buttons to **Zoom**, **Move** the reticle (Up, Down, Left, Right), adjust the size of the inner and outer circles, and toggle the onboard **Flash LED**.
   - Your custom settings (reticle position, center, and radius) will be automatically saved directly into the ESP32-CAM's permanent memory!
