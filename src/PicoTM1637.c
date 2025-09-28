#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <hardware/clocks.h>
#include <PicoTM1637.h>
#include <PicoTM1637.pio.h>

#define SET_WRITEMODE 0x40
#define WRITE_ADDRESS 0xc0
#define BRIGHTNESS_BASE 0x88
#define MAX_DIGITS 4

/* Global variables */
//PIO pio;
//uint clkPin, dioPin, sm, brightness = 0;
//bool colon = true;
//pio_sm_config smConfig;

static const uint8_t digitToSegment[] = {
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001     // F
  };

static const uint8_t segmentsArr[] = {
#include "../data/char_table.txt"
};

void TM1637_init(TM1637_device* device) {
  // Choose which PIO and sm instance to use 
  // Our assembled program needs to be loaded into this PIO's instruction
  // memory. This SDK function will find a location (offset) in the
  // instruction memory where there is enough space for our program. We need
  // to remember this location!

  PIO pio = device->pio;
  uint clkPin = device->clk_pin, dioPin = device->dio_pin, sm = device->sm;
  pio_sm_config* smConfig = &device->config;

  uint offset = pio_add_program(pio, &tm1637_program);

  *smConfig = tm1637_program_get_default_config(offset);

  gpio_pull_up(clkPin);
  gpio_pull_up(dioPin);

  pio_gpio_init(pio, clkPin);
  pio_gpio_init(pio, dioPin);

  sm_config_set_sideset_pins(smConfig, clkPin);

  uint32_t both_pins = (1u << clkPin) | (1u << dioPin);
  pio_sm_set_pins_with_mask(pio, sm, both_pins, both_pins);
  pio_sm_set_pindirs_with_mask(pio, sm, both_pins, both_pins);

  sm_config_set_out_pins(smConfig, dioPin, 1);
  sm_config_set_set_pins(smConfig, dioPin, 1);

  sm_config_set_out_shift(smConfig, true, false, 32);

  TM1637_refresh_frequency(device);

  // Load our configuration, and jump to the start of the program
  pio_sm_init(pio, sm, offset, smConfig);

  // Set the state machine running
  pio_sm_set_enabled(pio, sm, true);
}

void set_display_on(TM1637_device* device) {
  pio_sm_put_blocking(device->pio, device->sm, BRIGHTNESS_BASE + device->brightness);
}

void TM1637_put_2_bytes(TM1637_device* device, uint start_pos, uint data) {
  uint address = WRITE_ADDRESS + start_pos;
  pio_sm_put_blocking(device->pio, device->sm, (data << 16) + (address << 8) +  SET_WRITEMODE);
  set_display_on(device);
}

void TM1637_put_4_bytes(TM1637_device* device, uint start_pos, uint data) {
  uint address = WRITE_ADDRESS + start_pos;
  uint data1 = data & 0xffff;  // first two bytes
  uint data2 = data >> 16;     // last two bytes
  pio_sm_put_blocking(device->pio, device->sm, (data1 << 16) + (address << 8) + SET_WRITEMODE);
  pio_sm_put_blocking(device->pio, device->sm, data2 << 16);  // I have no idea why this has to be shifted
  set_display_on(device);
}

/* Convert a number to something readable for the 'put bytes' functions.
 *
 * Warning, input must not be more than 4 digits. Then least significant digits
 * will be cut of.
 *
 * You can also cut of parts with a bitmask. If bitMask = 0 nothing will
 * happen.*/
unsigned int num_to_hex(int num, uint bitMask) {
  unsigned int hex = 0x0, seg;
  if (num == 0) {
    // singular case
    hex = digitToSegment[0];
  } else {
    while (num) {
      seg = digitToSegment[num % 10];  // extract last digit as 7 segment byte
      num /= 10;  // remove last digit from num
      hex = seg + (hex << 8);  // Put new segment to the right in hex
    }
  }
  if (bitMask) {
    hex &= bitMask;
  }
  return hex;
}


/* Helper for converting a char to the 7-segment representation. */
uint fetch_char_encoding(char charToFind) {
  short i = 0;
  uint8_t c = 1;
  while (c != '\0') {
    c = segmentsArr[i];
    if (c == charToFind) {
      return segmentsArr[i+1];
    }
    i += 2;  // only every other element is a characer to look for
  }
  // Could not find charToFind, try to find lower/upper case of same letter.
  // note that every english letter exists in one of these forms.
  if ((0x40 < charToFind) && (charToFind < 0x5b)) {
    // this is upper case
    return fetch_char_encoding(charToFind + 0x20);
  } else if ((0x60 < charToFind) && (charToFind < 0x7b)) {
    // this is lower case
    return fetch_char_encoding(charToFind - 0x20);
  }
  // Still not found, return empty
  printf("Warning! char %c not found by fetch_char_encoding()\n", charToFind);
  return fetch_char_encoding(' '); 
}

void TM1637_display(TM1637_device* device, int number, bool leadingZeros) { 
  // Is number positive or negative?
  int isPositive;
  int isNegative;
  if (number >= 0) {
    isPositive = 1;
  } else {
    isPositive = 0;
    number = -1*number;
  }
  // Determine length of number
  int len = 0;
  int numberCopy = number;
  while (numberCopy) {
    len++;
    numberCopy /= 10;
  }
  if (len > 3 + isPositive) {
    printf("Warning number %d too long\n", number);
    len = 3 + isPositive;
    // least signigicant digits will be lost
  }

  // Get hex
  unsigned int hex = num_to_hex(number, 0);
  if (!isPositive) {
    hex = (hex << 8) + 0x40;  // add a negative sign
    len++;  // count negative sign in length
  } 
  unsigned int startPos = 0;
  if (leadingZeros && (len < MAX_DIGITS)) {
    for (int i=len; i<MAX_DIGITS; i++) {
      hex = (hex << 8) + digitToSegment[0];
    }
  } else if (number == 0) {
    // Signular case
    hex = digitToSegment[0];
    startPos = MAX_DIGITS - 1;
  } else { 
    startPos = MAX_DIGITS - len;
  }
  
  // Display number
  TM1637_put_4_bytes(device, startPos, hex);
}

void TM1637_display_word(TM1637_device* device, const char *word, bool leftAlign) {
  // Find the binary representation of the word
  uint bin = 0;
  int i = 0;
  char c = word[0];
  int col = -1;
  int len = 0;
  while ((c != '\0') && (len < MAX_DIGITS)) {
    if (c == ':') {
      // remember colon
      col = i - 1;
      if ((col < 0) || (col > MAX_DIGITS)) {
        printf("Warning, TM1637_display_word colon out of bounds.\n");
      }
    } else {
      bin += (fetch_char_encoding(c)<< 8*len);
      len++;
    }
    i++;
    c = word[i];
  }
  // Display word
  uint startIndex;
  if (leftAlign) {
    startIndex = 0;
  } else {
    startIndex = MAX_DIGITS - len;
  }
  // add colon
  if (col >= 0) {
    bin |= (0x80 << col*8);
  }
  TM1637_put_4_bytes(device, startIndex, bin);
}

void TM1637_display_moving_text(TM1637_device** devices, size_t device_count, const char* text, uint interval_ms, bool right_incoming)
{
  // Total empty digits to either the left and right of text. x4 for 4 digits per display.
  int n_spaces = device_count * 4;

  // Total iterations needed for the display of text. Includes the empty spaces.
  size_t n_char = strlen(text);
  int n_iterations = n_spaces * 2 + n_char;

  char all[n_iterations];

  // Set begin and end partitions with empty spaces, fill the remaining indecies with the input text.
  // Example with two displays: 
  // "_ _ _ _ _ _ _ _ t e s t _ _ _ _ _ _ _ _"
  memset(all, ' ', n_iterations);
  memcpy(all + n_spaces, text, n_char);

  if (right_incoming) // Shift left
  {
    // Only shift for spaces on one side + char count. Ex: "_ _ _ _ _ _ _ _ t e s t"
    for (int i_text = 0; i_text < (n_iterations - n_spaces); i_text++)
    {
      memmove(all, all + 1, (n_iterations - 1) * sizeof(char));
      all[n_iterations - 1] = ' ';
      for (int i_display = 0; i_display < device_count; i_display++)
        TM1637_display_word(devices[i_display], all + i_display * 4, false);
      sleep_ms(interval_ms);
    }
  }
  else
  {
    for (int i_text = 0; i_text < (n_iterations - n_spaces); i_text++)
    {
      memmove(all + 1, all, (n_iterations - 1) * sizeof(char));
      all[0] = ' ';
      for (int i_display = 0; i_display < device_count; i_display++)
        TM1637_display_word(devices[i_display], all + i_display * 4, false);
      sleep_ms(interval_ms);
    }
  }
}

/* Helper for getting the segment representation for a 2 digit number. */
uint two_digit_to_segment(TM1637_device* device, int num, bool leadingZeros) {
  uint hex = num_to_hex(num, 0xffff);

  int numDiv = num / 10;  // determine length of number
  
  if (!numDiv && leadingZeros) {
    // num is just 1 digit and we want leadning zeros
    hex = digitToSegment[0] + (hex << 8);
  } else if (!numDiv) {
    // num is just 1 digit  
    hex = hex << 8;
  }

  if(device->colon) {
    hex |= 0x8000;
  }
  
  return hex;
}

void TM1637_display_left(TM1637_device* device, int num, bool leadingZeros) {
  uint hex = two_digit_to_segment(device, num, leadingZeros);  
  TM1637_put_2_bytes(device, 0, hex);
}

void TM1637_display_right(TM1637_device* device, int num, bool leadingZeros) {
  uint hex = two_digit_to_segment(device, num, leadingZeros);
  TM1637_put_2_bytes(device, 2, hex);
}

void TM1637_display_both(TM1637_device* device, int leftNum, int rightNum, bool leadingZeros) {
  uint leftHex = two_digit_to_segment(device, leftNum, leadingZeros);
  uint rightHex = two_digit_to_segment(device, rightNum, leadingZeros);  

  uint hex = leftHex + (rightHex << 16);
  TM1637_put_4_bytes(device, 0, hex);
}

void TM1637_clear(TM1637_device* device) {
  pio_sm_put_blocking(device->pio, device->sm, 0x80);
  pio_sm_put_blocking(device->pio, device->sm, 0xc040);
  pio_sm_put_blocking(device->pio, device->sm, 0x0);
}

void TM1637_refresh_frequency(TM1637_device* device) {
  // Set sm clock close to 45 kHz
  uint32_t sysFreq = clock_get_hz(clk_sys);
  float divider = sysFreq/45000;
  if (divider > 65536) {
    divider = 65536;
  } else if (divider < 1) {
    divider = 1;
  }

  sm_config_set_clkdiv(&device->config, divider); 
}

void TM1637_wait(TM1637_device* device) {
  while (!pio_sm_is_tx_fifo_empty(device->pio, device->sm)) {
    // Wait while there is something in tx fifo (on the way out). 
    if (!pio_sm_is_rx_fifo_empty(device->pio, device->sm)) {
      // Found some response from the sm. Since the tx still isn't empty
      // we just want to throw this away.
      pio_sm_get_blocking(device->pio, device->sm);
    }
  }
  int d = 0;
  while (d != 1) {
    // Now there is nothing in tx fifo, but there might still be work to do.
    // The status 1 is sent as a response when done.
    d = pio_sm_get_blocking(device->pio, device->sm);
  }
  uart_default_tx_wait_blocking();
}
