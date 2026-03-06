# User Guide - ESP32-Cam Collimation Tool

Welcome to the ESP32-Cam Collimation Tool user guide! This document explains how to use the web interface to align your equipment accurately using the live camera feed and adjustable collimation reticles.

## Interface Overview 🖥️

When you log into the web interface, you'll see a live camera feed taking up the majority of the screen, with various controls located underneath. The screen is designed to be clean and simple, providing just the essential tools for collimation.

### The Camera Feed & Reticles
*   **Live Stream:** The video feed is automatically stretched and centered perfectly on your screen. The application outputs at *320x240 (QVGA)* in Grayscale for maximized framerates, essential for real-time adjusting.
*   **Crosshairs:** 
    *   A static vertical and horizontal line intersecting directly over the camera’s mechanical center point.
*   **Collimation Circles:**
    *   **Inner Circle (Solid Green):** Used for aligning closer or smaller elements.
    *   **Outer Circle (Dashed Green):** Used for wider alignments or defining an outer bounding edge.

## Controls & Adjustments ⚙️

Below the camera feed, you'll find a series of control panels to fine-tune your collimation layout.

### 1. Zoom Panel 🔍
*   Use the `+` and `-` buttons to digitally zoom in and out of the center of your camera feed.
*   The current zoom multiplier is displayed above the buttons (starting at `1.0x` and maxing out at `10.0x`).
*   *Note: Zooming is purely digital. It scales up the current resolution, it does not change the camera sensor properties.*

### 2. Position Panel 🎯
This panel allows you to shift the center point of both the Crosshairs and the Circles. This is crucial if your camera sensor is slightly misaligned from the mechanical center of your optical tube or assembly!
*   **UP / DN (Down):** Move the reticles across the vertical Y-axis.
*   **L (Left) / R (Right):** Move the reticles across the horizontal X-axis.
*   **Rst (Reset):** Instantly snaps the reticles back to the exact dead-center default coordinate (0, 0 / 160, 120).

### 3. Inner Circle Panel 🟢
Adjust the diameter of the solid green inner circle to match the specific ring or element you are attempting to collimate.
*   **+ Button:** Increases the radius by 5 pixels per click.
*   **- Button:** Decreases the radius by 5 pixels per click.
*   The current radius value is displayed. Minimum size is 5 pixels.

### 4. Outer Circle Panel ⭕
Adjust the diameter of the dashed green outer circle to match an outer bounding ring or secondary collimation element.
*   **+ Button:** Increases the radius by 5 pixels per click.
*   **- Button:** Decreases the radius by 5 pixels per click.
*   The current radius value is displayed. Minimum size is 5 pixels.

### 5. Hardware Panel 💡
*   **FLASH Button:** Toggles the extremely bright onboard LED flash located on the ESP32-CAM module. Click once to turn it on, click again to turn it off. Useful if working in pitch-black conditions. **Do not look directly at the LED!**

## Persistent Settings 💾
This application utilizes the ESP32's non-volatile storage (NVS) memory. This means **all of your adjustments are saved automatically.**
Every time you move the crosshairs, tweak a radius, or adjust a setting, it is immediately saved. If you lose power or disconnect the ESP32-CAM, the next time you turn it on and log in, your custom reticle positions and sizes will be exactly where you left them!
