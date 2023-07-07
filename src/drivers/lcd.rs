use alloc::vec;
use embedded_hal::blocking::delay::{DelayMs, DelayUs};
use embedded_hal::digital::v2::OutputPin;
use embedded_hal::blocking::i2c::Write;

pub enum Direction {
    LEFT,
    RIGHT,
}

pub enum DisplayState {
    ON,
    OFF,
}

pub enum CursorDisplayState {
    ON,
    OFF,
}

pub enum CursorBlinkState {
    BLINK,
    SOLID,
}

pub enum WriteMode {
    NORMAL,
    REVERSE,
} 

pub enum ShiftState {
    ON,
    OFF,
}

pub struct LCDisplay {
    data_bus: bus,
}

pub trait DataBus {
    pub fn write(&mut self, byte: u8, data: bool) -> Result<()>;
} 

pub struct LCDDataBus {
    pub rs: OutputPin,
    pub en: OutputPin,
    pub d4: OutputPin,
    pub d5: OutputPin,
    pub d6: OutputPin,
    pub d7: OutputPin,
    pub delay: DelayMs<u8> + DelayUs<u16>,
}

impl LCDDataBus {
    pub fn new(rs: &mut OutputPin, en: &mut OutputPin, d4: &mut OutputPin, d5: &mut OutputPin, d6: &mut OutputPin, d7: &mut OutputPin, delay: &mut (DelayMs<u8> + DelayUs<u16>)) -> Self {
        return Self {
            rs,
            en,
            d4,
            d5,
            d6,
            d7,
            delay,
        }
    }
}

impl DataBus for LCDDataBus {
    fn write(&mut self, byte: u8, data: bool) -> Result<()> {
        if !data {
            // no data to write, disable recieve pin
            self.rs.set_low().map_err(|_| Error)?;
            return Ok(())
        } 

        // MARK: Test to see if 200 microseconds is sufficient, really i think that we just need to
        // wait for a clock cycle

        // turn on recieve pin, enable read mode
        self.rs.set_high().map_err(|_| Error)?;

        // write upper half of byte and enbale recieve pin
        self.write_upper(byte)?; 
        self.en.set_high().map_err(|_| Error)?;
        self.delay.delay_us(200u16);
        self.en.set_low().map_err(|_| Error)?;

        // write lower half of byte and enbale recieve pin
        self.write_lower(byte)?;
        self.en.set_high().map_err(|_| Error)?;
        self.delay.delay_us(200u16);
        self.en.set_low().map_err(|_| Error)?;

        // turn off recieve pin, disable read mode
        self.rs.set_low().map_err(|_| Error)?;

        return Ok(())
    }
}

impl LCDDataBus {
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

pub struct DisplayConfiguration {
    pub cursor_display: CursorDisplayState,
    pub cursor_blink: CursorBlinkState,
    pub display_state: DisplayState,
}

impl DisplayConfiguration {
    pub fn default() -> Self {
        return Self {
            cursor_display: CursorDisplayState::ON,
            cursor_blink: CursorBlinkState::BLINK,
            display_state: DisplayState::ON,
        }
    } 

    pub fn as_byte(&self) {
        let has_cursor_blink = match self.cursor_blink {
            CursorBlinkState::BLINK => 1u8,
            CursorBlinkState::SOLID => 0u8,
        };

        let has_cursor_display = match self.cursor_display {
            CursorDisplayState::ON => 2u8,
            CursorDisplayState::OFF => 0u8,
        }; 

        let has_display_state = match self.cursor_display{
            DisplayState::ON => 4u8,
            DisplayState::OFF => 0u8,
        }; 

        return 8 | has_cursor_blink | has_cursor_display | has_display_state
    }
}

pub struct DisplaySize {
    pub rows: u8,
    pub cols: u8,
}

impl DisplaySize {
    pub fn new(rows: u8, cols: u8) -> Self {
        return Self { rows, cols }
    } 

    pub fn default() -> Self {
        return Self { rows: 2, cols: 16 }
    }
}

pub struct WriteConfiguration {
    pub write_mode: WriteMode,
    pub shift_state: ShiftState
}

impl WriteConfiguration {
    pub fn new(write_mode: WriteMode, shift_state: ShiftState) -> Self {
        return Self { write_mode, shift_state }
    }

    pub fn default() -> Self {
        return Self { write_mode: WriteMode::NORMAL, shift_state: ShiftState::OFF }
    }

    pub fn as_byte(&self) -> u8 {
        let has_write_mode = match self.write_mode {
            WriteMode::NORMAL => 2,
            WriteMode::REVERSE => 0,
        };

        let has_shift_state = match self.shift_state {
            ShiftState::ON => 1,
            ShiftState::OFF => 0,
        };

        return 3 | has_write_mode | has_write_mode
    }
}
