# project-atlas
My first dive into creating firmware for a board that previously did not function. I will be comitting here from time to time.

## building
The commands for building and flashing the code in this project are as follows:
- `cargo build -Z build-std=core --target avr-atmega1284p.json --release`
- `avrdude -p m1284p -c arduino -P /dev/tty.usbserial-14130 -b 57600 -U flash:w:target/avr-atmega1284p/release/project-atlas.elf`

## resources
The resources I used to develop this project.

- [Atmega 1284p datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/doc8059.pdf)
- [LCD2004 datasheet](https://cdn-shop.adafruit.com/datasheets/TC2004A-01.pdf)
- [HAL for embedded rust](https://github.com/Rahix/avr-hal)
- [ANET 3D v1.7 Mainboard](https://github.com/ralf-e/ANET-3D-Board-V1.0/blob/master/ANET3D_Board_Schematic.pdf)
- [LCD 44780 Driver](https://github.com/JohnDoneth/hd44780-driver)

Special thanks to the maintainers of these projects.
