<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
-->
BCM
===

Experiments with the Broadcom BCM2838 ARM Peripherals

## Running under Raspberry Pi OS (Raspian)

In the following, you'll be editing `/boot/config.txt` in order to:
* disable audio
* disable serial/uart
* set core_freq to 250 MHz
* disable WiFi & Bluetooth (Optional)

Before doing so, you may want to make a backup, e.g..
```
sudo cp /boot/config.txt /boot/config.txt-ORIG
```

Edit `/boot/config.txt`, e.g., 
```
sudo vi /boot/config.txt
```

**Set core frequency**
```
core_freq=250
```
This value is in MHz. Required if using UART.

**Disable audio**
```
dtparam=audio=off
```

**Disable UART/serial interface**
Disable kernels UART/serial interface (we'll be using our own)
```
enable_uart=0
```
Depending on the model, this might be under the `[all]` section.


**Disble WiFi & Bluetooth (Optional)**
Do this unless you really need them:
```
dtoverlay=disable-wifi
dtoverlay=disable-bt
```
This is typically in the main section.
