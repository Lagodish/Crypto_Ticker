# Crypto Ticker

This program will display current cryptocurrency prices on a small display. Price information is acquired from cryptocompare.com over WiFi or bluetooth. The board is based around the ESP32 microcontroller, which integrates WiFi, Bluetooth, and BLE connectivity. The final product incorporates a 3D-printed case, lithium battery, touch button, OLED display, and main control board. The device is woken up with the touch button and subsequent presses will cycle through different currencies on the display.


### Dependencies 

* squix78/ESP8266_SSD1306 @ ^4.1.0
* bblanchon/ArduinoJson @  5.13.4

* Clone to the "Home/Arduino/libraries" directory.

### Issues

* Uploading code to some boards fails. Change the serial upload speed from 921600 to 115200 in the Arduino IDE.

## Built With

* PlatformIO

## Authors

* **Kevin Dolan** - *Initial work* - [kjdolan99](https://github.com/kjdolan99)
* **Aleksandr** - *Forked work / rebuild* - [Lagodish](https://github.com/lagodish)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
