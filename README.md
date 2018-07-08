# KNXduino - KNX bus compatible DIY device based on STM32F303 MCU with Arduino flavour

Testing HW: [Nucleo-F303RE](https://www.st.com/en/evaluation-tools/nucleo-f303re.html)

Testing IDE: [Visual Studio Code](https://code.visualstudio.com/) + [Arduino plugin](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino) + [Cortex-Debug plugin](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)

Testing Arduino Core: Additional URL https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json version 1.3.0

## Known issues

* Fix hardcoded `arduinoPath` path in `.vscode/c_cpp_properties.json` file
* Fix hardcoded `cortex-debug.openocdPath` path in `.vscode/settings.json` file
* Fix hardcoded `cortex-debug.armToolchainPath` path in `.vscode/settings.json` file
* Fix hardcoded path to `stm32f3discovery.cfg` in `.vscode/launch.json` file
* `Arduino: Upload` command does not work because of `output` option in `.vscode/arduino.json` file. Use `Arduino: Verify` followed by `Debug: Continue (F5)` instead.

## Acknowledgement

This work is supported by [hkfree.org](http://www.hkfree.org) community network.