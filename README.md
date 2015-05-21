# Micros Micros - 8051F310 Micro [![Build Status](https://travis-ci.org/Viq111/micros-micros.svg?branch=master)](https://travis-ci.org/Viq111/micros-micros)

Micros Micros is a project to turn enable any table with a touch-sensitive interface using only sound propagation.
Required hardware is two micros. Related software is available here.

[![micro_video](https://cloud.githubusercontent.com/assets/2376565/7741011/9c62b4a6-ff47-11e4-8b7c-58ae092a28a1.png)](https://vid.me/CAXX)

## Installation

You first need a 8051F310 µChip. Plug the first micro to pin 0.0 and second micro to pin 0.1.

You can then flash the 8051F310 with the provided main file

## Basic Usage

You can talk with the setup using a serial interface at a baudrate of `56700`.

`a`: Test mode for first micro, the LED blink when a level is triggered. The LED should not be triggered when you are talking but should when you are gently tapping on the table.

`b`: same test mode for the second micro.

`r`: runmode, normal mode. This will print the time difference when micros micros senses something (a tap).

## Virtual Keyboard Usage

You need a Python2 interpreter and [pip](https://pip.pypa.io/en/latest/installing.html) installed.

Install the required dependancies:

- [pyserial](https://pypi.python.org/pypi/pyserial): `pip install pyserial`
- [pywin32](http://sourceforge.net/projects/pywin32/): [here](http://sourceforge.net/projects/pywin32/files/pywin32/)

You then need to run `virtual_keyboard.py`

```
cd py
python virtual_keyboard.py
```

The first step is to calibrate the device for each key input: select your key, then tap on the table until you go to the next key.
Press return to enter normal operating mode which will simulate keypresses you calibrated.
