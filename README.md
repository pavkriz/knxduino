# KNXduino - KNX bus compatible DIY device based on STM32F303 MCU with Arduino flavour

* This project is based on [Selfbus project](http://www.selfbus.org)

## Testing Board

* [Nucleo-F303RE](https://www.st.com/en/evaluation-tools/nucleo-f303re.html)
* Connections:
    * KNX RX voltage divider 33k:2k -> PB0 (COMP4_INP)
    * PB1 (COMP4_OUT) -> PA6 (TIM3_CH1)
    * PA7 (TIM3_CH2) -> TX MOSFET (shorting 68R to GND, 10k pull-down on gate)
    * KNX- <-> GND

## Development Environment


Testing IDE: [Visual Studio Code](https://code.visualstudio.com/) + [Arduino plugin](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino) + [Cortex-Debug plugin](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)

Testing Arduino Core: Additional URL https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json version 1.3.0

Currently compiles to 31732 bytes of Flash (no bootloader used yet). Thus perfectly OK for 128kB devices.

### Bootloader

Use Atollic TrueSTUDIO for STM32

## Known issues and steps to make it work

* Copy `misc/boards.local.txt` to the folder, where STM32duino Core is installed (eg. `/home/pavkriz/.arduino15/packages/STM32/hardware/stm32/1.3.0/`)
* Fix hardcoded `arduinoPath` path in `.vscode/c_cpp_properties.json` file
* Fix hardcoded `cortex-debug.openocdPath` path in `.vscode/settings.json` file
* Fix hardcoded `cortex-debug.armToolchainPath` path in `.vscode/settings.json` file
* Fix hardcoded path to `stm32f3discovery.cfg` in `.vscode/launch.json` file
* `Arduino: Upload` command does not work because of `output` option in `.vscode/arduino.json` file. Use `Arduino: Verify` followed by `Debug: Continue (F5)` instead.

## TODO

* Migrate to PlatformIO.org when it will fully support official ST's STM32duino Core (which we use here). PlatformIO.org has much better build and dependency management (configurable preprocessor defines, local references to particular Arduino Core, libraries,...). Watch https://github.com/platformio/platform-ststm32/issues/76

## Acknowledgement

This work is supported by [hkfree.org](http://www.hkfree.org) community network.