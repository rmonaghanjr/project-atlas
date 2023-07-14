#![no_std]
#![no_main]

#![feature(alloc_error_handler)]

extern crate atmega_hal;
extern crate avr_device;
extern crate avr_hal_generic;
extern crate embedded_hal;
extern crate nb;
extern crate ufmt;

mod drivers;

use atmega_hal::port;
use atmega_hal::prelude::*;
use atmega_hal::usart::BaudrateExt;
use avr_device::atmega1284p;

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[atmega_hal::entry]
fn main() -> ! {
    let mut _delay = atmega_hal::delay::Delay::<atmega_hal::clock::MHz16>::new();
    let peripherals = atmega_hal::Peripherals::take().unwrap();
    let pins = atmega_hal::pins!(peripherals);

    // d7 = pc1
    // d6 = pc00
    // d5 = pd3
    // d4 = pd2
    // en = pa2
    // rs = pa3

    let usb_rx = pins.pd0;
    let usb_tx = pins.pd1.into_output();
    let mut onboard_led = pins.pa4.into_output();

    let serial = atmega_hal::Usart::new(
        peripherals.USART0,
        usb_rx,
        usb_tx,
        115200.into_baudrate::<atmega_hal::clock::MHz16>(),
    )
    .split();

    let mut reader: atmega_hal::usart::UsartReader<
        atmega1284p::USART0,
        port::Pin<port::mode::Input, port::PD0>,
        port::Pin<port::mode::Output, port::PD1>,
        atmega_hal::clock::MHz16,
    > = serial.0;

    let mut writer: atmega_hal::usart::UsartWriter<
        atmega1284p::USART0,
        port::Pin<port::mode::Input, port::PD0>,
        port::Pin<port::mode::Output, port::PD1>,
        atmega_hal::clock::MHz16,
    > = serial.1;

    ufmt::uwriteln!(&mut writer, "cmd interface ready\r").void_unwrap();
    let mut command: [char; 16] = ['\0'; 16];
    let mut curr_char: usize = 0;
    loop {
        let byte = nb::block!(reader.read()).void_unwrap();

        if byte == 13 {
            let led_on_cmd: [char; 16] = ['l', 'e' , 'd' , ' ' , 'o' , 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'];
            let led_off_cmd: [char; 16] = ['l' , 'e' , 'd' , ' ' , 'o' , 'f' , 'f', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'];

            if command == led_on_cmd[..] {
                onboard_led.set_high();
                ufmt::uwriteln!(&mut writer, "led on\r").void_unwrap();
            } else if command == led_off_cmd[..] {
                onboard_led.set_low();
                ufmt::uwriteln!(&mut writer, "led off\r").void_unwrap();
            } else {
                ufmt::uwriteln!(&mut writer, "unknown command\r").void_unwrap();
            }

            command = ['\0'; 16];
            curr_char = 0;
        } else {
            if curr_char == 16 {
                ufmt::uwriteln!(&mut writer, "warning: command buffer overflow\r").void_unwrap();
                curr_char = 0;
            }

            command[curr_char] = byte as char;
            curr_char += 1;
        }
    }
}
