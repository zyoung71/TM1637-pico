#include <stdlib.h>
#include <stdio.h>
#include <PicoTM1637.h>
#include <pico/stdlib.h>
#define CLK_PIN 27
#define DIO_PIN 28

int main()
{   
    TM1637_device device2 = {
      .brightness = 7,
      .clk_pin = CLK_PIN,
      .dio_pin = DIO_PIN,
      .sm = 1,
      .colon = 1,
      .pio = pio1
    };
    TM1637_device device = {
      .brightness = 7,
      .clk_pin = 6,
      .dio_pin = 7,
      .sm = 0,
      .colon = 1,
      .pio = pio0
    };

    TM1637_init(&device);  
    TM1637_clear(&device); 
    TM1637_init(&device2);  
    TM1637_clear(&device2); 

    TM1637_display_word(&device, "dEMO", true);
    TM1637_display_word(&device2, "dEMO", true);
    sleep_ms(2000);

    TM1637_put_4_bytes(&device, 1, 0x4f5b06);  // raw bytes for 123 
    TM1637_put_4_bytes(&device2, 1, 0x4f5b06);  // raw bytes for 123 
    sleep_ms(1000);
    TM1637_put_4_bytes(&device, 1, 0x4f5b06); // something new needs to be displayed.
    TM1637_put_4_bytes(&device2, 1, 0x4f5b06); // something new needs to be displayed.
    sleep_ms(1000);
    
    TM1637_clear(&device);
    TM1637_clear(&device2);
    sleep_ms(500);
    
    printf("DEMO\n");
    // Count down from 150 to -50
    int count = 150;
    TM1637_display(&device, count, false);
    TM1637_display(&device2, count, false);
    sleep_ms(500);
    while(count >= -50) {
      TM1637_display(&device, count, false);
      TM1637_display(&device2, count, false);
      count--;
      // The display can not update too often. So even though there is no
      // sleep, this will take a couple of moments.
    }
    
    sleep_ms(1000);
    TM1637_clear(&device);
    TM1637_clear(&device2);
    sleep_ms(500);

    // Demo a clock, by default there will be a colon between the numbers.
    int seconds = 0;
    int minutes = 0;
    TM1637_display_both(&device, minutes, seconds, true);
    TM1637_display_both(&device2, minutes, seconds, true);
    while (true) {
      sleep_ms(1000);
      seconds++;
      if (seconds == 60) {
        seconds = 0;
        minutes++;
        TM1637_display_both(&device, minutes, seconds, true);
        TM1637_display_both(&device2, minutes, seconds, true);
      } else {
        TM1637_display_right(&device, seconds, true);
        TM1637_display_right(&device2, seconds, true);
      }
    }

    return 0;
} 
