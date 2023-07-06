use alloc::vec;
use embedded_hal::blocking::delay::{DelayMs, DelayUs};
use embedded_hal::blocking::i2c::Write;
use embedded_hal::digital::v2::OutputPin;

pub struct LCDDataBus<rs: OutputPin, en: OutputPin, d4: OutputPin, d5: OutputPin, d6: OutputPin, d7: OutputPin> {
    rs: rs,
    en: en,
    d4: d4,
    d5: d5,
    d6: d6,
    d7: d7,
}

impl<rs: OutputPin, en: OutputPin, d4: OutputPin, d5: OutputPin, d5: OutputPin, d6: OutputPin, d7: OutputPin>LCDDataBus<rs, en, d4, d5, d6, d7> {
    fn write_lower(&mut self, data: u8) -> Result<()> {
        let pins = vec![self.d4, self.d5, self.d6, self.d7];

        pins
            .iter()
            .enumerate()
            .for_each(|(i, pin)| 
                if data & (0b0001 << i) {
                    pin.set_high() 
                } else {
                    pin.set_low()
                }
            );

        return Ok(())
    }
    
    fn write_upper(&mut self, data: u8) -> Result<()> {
        let pins = vec![self.d4, self.d5, self.d6, self.d7];

        pins
            .iter()
            .enumerate()
            .for_each(|(i, pin)| 
                if data & (0b00010000 << i) {
                    pin.set_high() 
                } else {
                    pin.set_low()
                }
            );

        return Ok(())
    }
}


