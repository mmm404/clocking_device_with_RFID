# RFID Clocking Device – Arduino Mega 2560 + ESP32
<img width="1211" height="596" alt="Screenshot from 2025-07-31 17-03-38" src="https://github.com/user-attachments/assets/22dd1a76-b595-4ad0-ad13-59de0e4c2e6a" />
This project implements a secure and user-friendly RFID-based employee/staff clocking system using the **Arduino Mega 2560**, **ESP32**, and **Arduino Pro Mini**. It logs clock-in and clock-out events via RFID tags, stores data in EEPROM, and provides real-time interaction using a TFT LCD screen.

## ## Features

* RFID access control using MFRC522 module  
* Real-time clocking interface on TFT LCD screen  
* EEPROM-based ID storage for offline verification  

* Admin management interface for enrolling and deleting IDs  
* Serial communication between:
  * Arduino Mega (RFID & logic)
  * Arduino Pro Mini (TFT LCD display UI)
  * ESP32 (future extension for Wi-Fi or cloud sync)
* Buzzer and LED feedback for user confirmation  
* Supports clock-in and clock-out with timestamp logic

## ## Hardware Used

* Arduino Mega 2560 (main controller)  
* Arduino Pro Mini (handles TFT LCD display UI)  
* ESP32 (for future communication and connectivity features)  
* RC522 RFID module  
* 2.4” or 2.8” TFT LCD display (ILI9341)  
* EEPROM or internal Arduino memory  
* Push buttons (e.g., reset or override)  
* LEDs + buzzer for status feedback  
* Power supply or USB  

## ## How It Works

1. **Scan Tag:** User scans an RFID tag using the MFRC522 reader.  
2. **ID Verification:** The Arduino Mega checks if the tag is in the EEPROM (enrolled).  
3. **Clocking Logic:**
   * If ID is valid and not clocked in → Clock-in is recorded.  
   * If ID is valid and already clocked in → Clock-out is recorded.  
4. **Feedback:**
   * Success or error messages are shown on the TFT LCD via the Pro Mini.  
   * Buzzer and LED signal confirm the action.  
5. **Admin Mode:**
   * Add or delete tags using special admin card or button sequence.  
6. **Serial Communication:** Arduino Mega sends display instructions to Pro Mini via serial. ESP32 can be used for Wi-Fi/BT communication or data backup.

## ## Code Structure

* `rfid_clocking.ino` → Main logic on Arduino Mega  
* `lcd_ui.ino` → Display driver and UI on Pro Mini  
* `esp32_link.ino` → Communication interface (optional)  

## ## Communication Protocol

```
Mega 2560 → Pro Mini:
[CMD],[MSG]

Examples:
INFO,Welcome Samuel
ERROR,Tag Not Registered
SUCCESS,Clock-in Success
```

## ## Future Improvements

* Integrate Wi-Fi or Bluetooth sync using ESP32 to sync data to Google Sheets or a server.  
* Use SD card or SPIFFS for large data logging.  
* Implement real-time clock (RTC) module for accurate timestamping.  

## ## UI Overview

* Main screen shows: Welcome message, time, status  
* Clock-in/clock-out feedback  
* Admin menu for tag enrollment  

## ## Folder Structure

```
/RFID_Clocking_Device
├── rfid_clocking.ino
├── lcd_ui.ino
├── esp32_link.ino (optional)
├── README.md
└── assets/ (icons, diagrams)
```

## ## Security Notes

* Tags are checked against stored unique IDs.  
* Admin functions protected via master card or password.  
* EEPROM write protection included.
