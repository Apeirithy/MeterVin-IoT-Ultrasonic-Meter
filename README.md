# MeterVin: IoT Ultrasonic Distance Meter

An ESP32-based contactless length measurement device that utilizes an HC-SR04 ultrasonic sensor. This physical computing project solves the limitations of conventional measuring tools by providing stable, non-contact measurements and automatically transmitting the results to a user's smartphone via Wi-Fi and WhatsApp.

<div align="center">
    <img src="./Assets/20241221_235642.Jng" alt="MeterVin Physical Product" width="40%">
</div>

## Hardware Architecture & Components
The system acts as a standalone portable measurement tool, integrating the following hardware:
* **Microcontroller:** ESP32 (handles sensor data processing, LCD UI, and Wi-Fi connectivity).
* **Sensor:** HC-SR04 Ultrasonic Sensor (calculates travel time of sound waves for accurate distance measurement).
* **Display:** 20x4 I2C LCD for the on-device user interface.
* **Inputs:** Tactile push buttons for UI navigation (Function & Select).
* **Power:** Portable Battery Shield for mobile operation.

> **Hardware Design:** [Click here to view the Block Diagram & Schematics](./Hardware_Design)

## Firmware Logic & Features
Developed in C++ (Arduino framework), the firmware implements a robust user interface and IoT communication:
* **Interactive LCD Menu:** Features a custom UI with debounced button inputs to navigate through Wi-Fi connection states, measurement displays, and sending options.
* **Device Size Calibration:** Allows users to dynamically include or exclude the device's physical length (12.2 cm offset) from the final measurement to ensure edge-to-edge accuracy.
* **Network Manager:** A built-in Wi-Fi reconnection sequence that automatically retries the connection or allows the user to skip and use the device purely offline.
* **WhatsApp API Integration:** Uses the `HTTPClient` library to construct and send a POST request via the CallMeBot API, delivering the formatted measurement data (in cm and meters) directly to the user's WhatsApp.

## System Flowchart
The operation flow ensures failsafe navigation. If Wi-Fi is unavailable, the user can bypass the connection phase and proceed to local measurement, preventing the device from being locked in a connection loop.
> [View the System Flowchart](./Docs/Flowchart_MeterVin.png)

## Repository Contents
* **[`/Firmware_src`](./Firmware_src):** The ESP32 C++ source code. *(Note: Copy `credentials_template.h`, rename to `credentials.h`, and input your Wi-Fi & CallMeBot API details).*
* **[`/Hardware_Design`](./Hardware_Design):** EasyEDA circuit schematics and system block diagrams.
* **[`/Docs`](./Docs):** Project presentation (PPT), evaluations, and logic flowcharts.
