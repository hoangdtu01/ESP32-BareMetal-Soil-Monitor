# 🌱 ESP32 Register-Level Irrigation System

A smart irrigation system developed on ESP32 using direct register programming (Bare-Metal), without Arduino libraries.

## Features

* Read soil moisture sensor using ESP32 ADC registers
* Display moisture percentage on LCD 16x2
* Control water pump through relay
* UART debugging via direct register access
* LCD driver implemented without external libraries

## Hardware

* ESP32-WROOM-32
* Soil Moisture Sensor
* LCD 16x2
* Relay Module
* Water Pump

## Technologies

* C/C++
* ESP32 Bare-Metal Programming
* Register-Level GPIO Control
* ADC Interface
* UART Communication

## System Flow

1. Read soil moisture value from ADC.
2. Convert raw ADC value to moisture percentage.
3. Display result on LCD.
4. Turn the pump ON when soil is dry.
5. Turn the pump OFF when soil moisture is sufficient.

## Author

Hoang Tran
