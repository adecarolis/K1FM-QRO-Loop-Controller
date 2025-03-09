# K1FM QRO Loop Controller  

![Controller Enclosure](https://github.com/adecarolis/k1fm-qro-loop-controller/raw/main/images/controller-enclosure-7300.jpg)

## Introduction  

This controller enables the construction of a relatively inexpensive **Magnetic Loop Antenna** for HF bands, such as [this one](https://k1fm.us/2025/02/k1fm-qro-loop/).

## Specifications  

- **ESP32 processor**  
- **Control options:**  
  - Web interface  
  - LCD Display + Pushbuttons / Rotary Encoder  
- **Enhanced memory management:**  
  - Large number of memory slots  
  - Automatically sorted  
- **Automation features:**  
  - Automatic memory selection  
  - Automatic antenna tuning  
- **Dynamic configuration:**  
  - Set endstop dynamically via the web interface  
  - Configure WiFi and rigctld settings without modifying the source code  

## Hardware  

The project is inspired by [Jose's (EA7HVO) controller](https://www.instructables.com/Magnetic-Loop-Controller-for-4-Antennas), from which I borrowed the overall concept, enclosure, hardware UI, and numerous other insights.

Jose’s project is **excellent and well-proven**. If remote operation is not a priority for you, I highly recommend checking out his work.

All components match EA7HVO’s project, except:

1. The control board is a **Wemos D1 R32**, which has the same form factor as an **Arduino Uno** but is powered by an **ESP32** processor.

2. The **Protoneer CNC Shield 3.0** is the same board used in EA7HVO's project, however in this one you **must remove the only resistor on the board (R1)** in order to make it work. You can either cut the resistor using scissors (it is through hole) or you can desolder it.

3. Wires are connected in a different order:

    | Pin Name      | ESP32 GPIO | Protoneer CNC 3.0 Board Pin |  
    |--------------|-----------|----------------------------|  
    | SCL_PIN      | 26        | ZSTEP                      |  
    | SDA_PIN      | 17        | XSTEP                      |  
    | BUTTON1_PIN  | 13        | ENDSTOP X-                 |  
    | BUTTON2_PIN  | 5         | ENDSTOP Y-                 |  
    | BUTTON3_PIN  | 23        | ENDSTOP Z-                 |  
    | BUTTON4_PIN  | 14        | ZDIR                        |  
    | BUTTON5_PIN  | 18        | SPINDIR                     |  
    | ENCODER_PIN1 | 16        | XDIR                        |  
    | ENCODER_PIN2 | 19        | SPINENABLE                  |  


4. Unlike Jose's boards, all connections to the CNC shield can be made using pin headers without the need to solder anything (the only soldered wires are below the Wemos board, used for 12V power supply).

Apart from these modifications, everything else is the same. This ensures a **well-documented** build process while adding remote operation capabilities and improved memory management.


## Software  

 **Software has been rewritten from scratch**. Rotation is now handled by the AccelStepper
library which allows faster speeds and controlled acceleration/deceleration of the stepper. The biggest additions are the **web interface**, the improved **memory management**, and the **automation features**.

The entire project can be **emulated online** using [Wokwi](https://wokwi.com/projects/423139990419467265). However, note that the free Wokwi version does not support the web interface, nor the automation features.

Below is a screenshot of the web interface:

![Web Interface](https://github.com/adecarolis/k1fm-qro-loop-controller/raw/main/images/web-interface.png)

## Configuration
When powered on, the controller will start in Access Point mode. Connect to the *K1FM Loop Controller* Wi-Fi network and navigate to **192.168.4.1** in your browser to access the control interface. From there, you can operate the antenna directly. Additionally, you can configure the controller to connect to your home Wi-Fi and set up the rigctld server parameters, enabling the automation features.  

Once the Wi-Fi parameters are configured, the controller will automatically connect to your home Wi-Fi network on subsequent power-ups, bypassing Access Point mode.

### Endstop Configuration

You must set your capacitor's endstop setting using the LCD menu or the web interface. This is very important since accidentally rotating the shaft past either endstop could irreversibly damage your vacuum capacitor.

## Planned Improvements

- ~~automatic memory selection~~ DONE
- ~~automatic antenna tuning~~ DONE
- ~~dynamic endstop configuration~~ DONE
- ~~OTA configuration management (WiFi / rigctld)~~ DONE
- Generalize capacitance calculation to support other capacitors

## Disclaimer  

This project is provided as is, without any express or implied warranties.  
The author assumes no responsibility for any damage, malfunctions, or regulatory violations resulting from the use of this controller.  
**Use at your own risk.**

## License  

This project is licensed under the MIT License.  
You are free to use, modify, and distribute it, provided that the original license and attribution remain intact.