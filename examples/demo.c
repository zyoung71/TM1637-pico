#include <stdlib.h>
#include <stdio.h>
#include <PicoTM1637.h>
#include <pico/stdlib.h>

int main()
{   
    stdio_init_all();

    TM1637_device_t device = {
      .brightness = 7,
      .upsidedown = false,
      .clk_pin = 5,
      .dio_pin = 4,
      .sm = 0,
      .colon = 1,
      .pio = pio0
    };
    
    TM1637_device_t device2 = {
      .brightness = 7,
      .upsidedown = false,
      .clk_pin = 7,
      .dio_pin = 6,
      .sm = 1,
      .colon = 1,
      .pio = pio0
    };

    TM1637_device_t device3 = {
      .brightness = 7,
      .upsidedown = false,
      .clk_pin = 9,
      .dio_pin = 8,
      .sm = 2,
      .colon = 1,
      .pio = pio0
    };

    TM1637_init(&device);  
    TM1637_clear(&device); 
    TM1637_init(&device2);  
    TM1637_clear(&device2);
    TM1637_init(&device3);  
    TM1637_clear(&device3); 
    
    TM1637_device_t* d1ptr = &device;
    TM1637_device_t* d2ptr = &device2;
    TM1637_device_t* d3ptr = &device3;
    while (1)
    {
      
      TM1637_display_moving_text(&d1ptr, 1, "Fuck", 150, true);
      TM1637_display_moving_text(&d2ptr, 1, "gAy", 150, true);
      TM1637_display_moving_text(&d3ptr, 1, "ASS", 150, true);

      TM1637_display_word(d1ptr, "I", true);
      sleep_ms(1000);
      TM1637_display_word(d2ptr, "HAVE", true);
      sleep_ms(1000);
      TM1637_display_word(d3ptr, "A", true);
      sleep_ms(1500);
      for (int i = 0; i < 4; i++)
      {
        TM1637_display_word(d1ptr, "bomb", true);
      TM1637_display_word(d2ptr, "bomb", true);
      TM1637_display_word(d3ptr, "bomb", true);
      sleep_ms(500);
      TM1637_clear(d1ptr);
      TM1637_clear(d2ptr);
      TM1637_clear(d3ptr);
      sleep_ms(500);
      }
    }

    TM1637_display_word(&device, "dEMO", true);
    TM1637_display_word(&device2, "dEMO", true);
    TM1637_display_word(&device3, "dEMO", true);
    sleep_ms(10000);
    
    TM1637_put_4_bytes(&device, 1, 0x4f5b06);  // raw bytes for 123 
    TM1637_put_4_bytes(&device2, 1, 0x4f5b06);  // raw bytes for 123 
    TM1637_put_4_bytes(&device3, 1, 0x4f5b06);  // raw bytes for 123 
    sleep_ms(1000);
    device.brightness = 0;
    device2.brightness = 0;
    TM1637_put_4_bytes(&device, 1, 0x4f5b06); // something new needs to be displayed.
    TM1637_put_4_bytes(&device2, 1, 0x4f5b06); // something new needs to be displayed.
    TM1637_put_4_bytes(&device3, 1, 0x4f5b06); // something new needs to be displayed.
    sleep_ms(1000);
    device.brightness = 7;
    device2.brightness = 7;
    device3.brightness = 7;
    
    TM1637_clear(&device);
    TM1637_clear(&device2);
    TM1637_clear(&device3);
    sleep_ms(500);
    
    printf("DEMO\n");
    // Count down from 150 to -50
    int count = 150;
    TM1637_display(&device, count, false);
    TM1637_display(&device2, count, false);
    TM1637_display(&device3, count, false);
    sleep_ms(500);
    while(count >= -50) {
      TM1637_display(&device, count, false);
      TM1637_display(&device2, count, false);
      TM1637_display(&device3, count, false);
      count--;
      // The display can not update too often. So even though there is no
      // sleep, this will take a couple of moments.
    }
    
    sleep_ms(1000);
    TM1637_clear(&device);
    TM1637_clear(&device2);
    TM1637_clear(&device3);
    sleep_ms(500);

    TM1637_display_word(&device, "HELL", true);
    TM1637_display_word(&device2, "O TH", true);
    TM1637_display_word(&device3, "ERE ", true);
    sleep_ms(2000);
    TM1637_clear(&device);
    TM1637_clear(&device2);
    TM1637_clear(&device3);

    TM1637_device_t* devices[] = {&device, &device2, &device3};
    char moving_text[] = "Hello World";
    TM1637_display_moving_text(devices, count_of(devices), moving_text, 200, true);

    // Demo a clock, by default there will be a colon between the numbers.
    int seconds = 0;
    int minutes = 0;
    TM1637_display_both(&device, minutes, seconds, true);
    TM1637_display_both(&device2, minutes, seconds, true);
    TM1637_display_both(&device3, minutes, seconds, true);
    while (true) {
      sleep_ms(1000);
      seconds++;
      if (seconds == 60) {
        seconds = 0;
        minutes++;
        TM1637_display_both(&device, minutes, seconds, true);
        TM1637_display_both(&device2, minutes, seconds, true);
        TM1637_display_both(&device3, minutes, seconds, true);
      } else {
        TM1637_display_right(&device, seconds, true);
        TM1637_display_right(&device2, seconds, true);
        TM1637_display_right(&device3, seconds, true);
      }
    }

    return 0;
} 
