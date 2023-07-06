#![no_std]
#![no_main]

extern crate atmega_hal;
extern crate embedded_hal;
extern crate hd44780_driver;

mod drivers;

use core::borrow::Borrow;

use atmega_hal::prelude::*;
use atmega_hal::usart::*;
use crate::drivers;

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[atmega_hal::entry]
fn main() -> ! {
    let mut delay = atmega_hal::delay::Delay::<atmega_hal::clock::MHz16>::new();
    let peripherals = atmega_hal::Peripherals::take().unwrap();
    let pins = atmega_hal::pins!(peripherals);

    // d7 = pc1
    // d6 = pc0
    // d5 = pd3
    // d4 = pd2
    // en = pa2
    // rs = pa3

    let mut rs = pins.pa3.into_output();
    let mut en = pins.pa2.into_output();
    let mut d4 = pins.pd2.into_output();
    let mut d5 = pins.pd3.into_output();
    let mut d6 = pins.pc0.into_output();
    let mut d7 = pins.pc1.into_output();

    let mut brightness = pins.pc2.into_pull_up_input();

    let mut lcd = hd44780_driver::HD44780::new_4bit(rs, en, d4, d5, d6, d7, &mut delay).unwrap();
    lcd.set_display_mode(hd44780_driver::DisplayMode {
        display: hd44780_driver::Display::On,
        cursor_visibility: hd44780_driver::Cursor::Visible,
        cursor_blink: hd44780_driver::CursorBlink::On
    }, &mut delay);

    loop {
        lcd.write_str("TEST DATA", &mut delay);

        delay.delay_ms(1000u16);
    }
}

