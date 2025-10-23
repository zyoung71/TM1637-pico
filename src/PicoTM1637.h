/** @file */

#ifndef TM1637_H_
#define TM1637_H_

#include <hardware/pio.h>
#include <pico/types.h>

/**
 * Device date type for all TM1637 functions. */
typedef struct TM1637_device
{
    PIO pio; // PIO instance
    uint clk_pin; // Clock
    uint dio_pin; // D i/o
    uint sm; // State machine
    uint brightness;
    bool colon;
    bool upsidedown;

    pio_sm_config _config;
    uint _pio_offset;
    
} TM1637_device_t;

/** 
 * Initiate TM1637 display
 *
 * @param device The device to use.
 * @param clk is the clock GPIO pin number. 
 * @param dio is the data GPIO pin number. */
void TM1637_init(TM1637_device_t* device);

/** Display one or two bytes of raw data on the display. 
 *
 * @param device The device to use.
 * @param startPos The digit index to start at. Ranges from `0` to `3`, where 
 *        `0` is to the left
 * @param data The data for one or two bytes, the least significant byte will be 
 *        put to the left. 
 *        
 * For example `TM1637_put_2_bytes(2, 0x3f05)` will show the number 10 on the
 * right half of the display. */
void TM1637_put_2_bytes(TM1637_device_t* device, uint startPos, uint data);

/** Display one to four bytes of raw data on the display. 
 *
 * @param device The device to use.
 * @param startPos The digit index to start at. Ranges from `0` to `3`, where 
 *        `0` is to the left
 * @param data The data for one to four bytes, the least significant byte will 
 *        be put to the left. */
void TM1637_put_4_bytes(TM1637_device_t* device, uint startPos, uint data);

/** Display a positive number with 4 digits or a negative number with 3 digits.
 * 
 *  @param device The device to use.
 *  @param number The number to display.
 *  @param leadingZeros If leading zeros should be displayed or not. */
void TM1637_display(TM1637_device_t* device, int number, bool leadingZeros);    
  
/** Display a string of characters.
 *
 * @param device The device to use.
 * @param word The word to display. May be at most 4 letters long.
 * @param leftAlign true if left alignment is desired, false for right 
 * alignment. Has no effect if all 4 chars are used. 
 *
 * All English alphabet letters are supported in lower or upper case. If
 * the desired case is not found, the other will be displayed instead.
 * If a character is not found at all it will be replaced by white space.
 * For a full list of supported characters, as well as their hexadecimal
 * representation please look at char_table.txt.
 *
 * You can also include a colon (:) in the string. This character is not
 * counted in the word length as the colon internaly belongs to the character
 * before it. Will only work if aligned with the colon spot on the display. */
void TM1637_display_word(TM1637_device_t* device, const char *word, bool leftAlign);

/**
 * Display animated text incoming from the left or right. Supports multiple devices for longer text.
 * 
 * @param devices The devices to use for this text.
 * @param device_count The count of devices.
 * @param text Input text for the animation.
 * @param char_count The amount of characters in text. Normally just `sizeof(text)`.
 * @param right_incoming true if text should display from right to left, false if left to right. */
void TM1637_display_moving_text(TM1637_device_t** devices, size_t device_count, const char* text, uint interval_ms, bool right_incoming);

/** Display a positive number on the 2 leftmost digits on the display. 
 *
 * A colon is by default shown. To turn this off use 
 * TM1637_set_colon(bool false).
 *
 * __Avoid using this function.__ It will cause the right side to flicker. 
 * Instead use TM1637_display_both(). */
void TM1637_display_left(TM1637_device_t* device, int number, bool leadingZeros);

/** Display a positive number on the 2 rightmost digits on the display. 
 * 
 * A colon is by default shown. To turn this off use 
 * TM1637_set_colon(bool false). */
void TM1637_display_right(TM1637_device_t* device, int number, bool leadingZeros);

/** Display two (2 digit positive) numbers on the display. 
 * 
 * A colon is by default shown in between. To turn this off use 
 * TM1637_set_colon(bool false). */
void TM1637_display_both(TM1637_device_t* device, int leftNumber, int rightNumber, bool leadingZeros);

/** Clear the display. */
void TM1637_clear(TM1637_device_t* device);

/** Reset the frequency at which TM1637 recives data.
 *
 * Call this function in case the system clock frequency has been changed since
 * the call of `TM1637_init()`. */
void TM1637_refresh_frequency(TM1637_device_t* device);

/** Wait for the TM1637 display.
 * 
 * When calling a function such as TM1637_display() the result is actually not
 * immediately sent to the display. This is because the PIO hardware on the pico
 * is running slower than the CPU. Usually this is fine, it's fast enough to
 * appear instant, but sometimes you want to wait for it to finish. For example
 * if you will enter sleep mode and want the display to update beforehand. */
void TM1637_wait(TM1637_device_t* device);

#endif // TM1637_H_
