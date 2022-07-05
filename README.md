## Programming
To flash your SMART Response XE you have 2 options:
1) Teardown the unit and access the ICSP standard pins (shown in the top left of the photo A)
2) Use POGO Pins and the hidden JTAG interface. You can follow this [link](https://www.hackster.io/news/run-basic-on-your-smart-response-xe-2041f035529d) for the details;

## Serial interface with Single Board Computer
The SMART Response XE (or, to be precise, the microcontroller ATMega1284P) does have 2 UART ports.
Unfortunately RX0 and TX0 cannot be used because are used by the Keyboard Matrix. (As we can see from this very useful [Schematic](https://github.com/fdufnews/SMART-Response-XE-schematics/blob/master/Smart_Response_XE.pdf) ).
So the only solution (if we don't want to lose any keys on the keyboard) is to use RX1 and TX1.

This choose come with it's own drawbacks.
TX1 is connected to the CS line of the onboard SPI EEPROM.

But for this specific application we don't need that, so we proceed by removing R3 and connecting our TX Line to the left pad of R3 (yellow wire in photo B).
For the RX Line we solder it to the left pad of R1 (which goes to the On/Off push buttons).

In my precise application I've mounted an after market power-bank inside the case. So, to turn the power bank On / Off, I've used the already present push button.
To avoid turning off the SMART Response XE, the component named Z1 was removed.

## Photo A
![IMG_2487.png](./DOCS/IMG_2487.png)

## Photo B
![IMG_2488.png](./DOCS/IMG_2488.png)

## Special thanks to
- <https://www.hackster.io/news/run-basic-on-your-smart-response-xe-2041f035529d>
- <https://github.com/fdufnews/SMART-Response-XE-schematics>
- <https://github.com/bitbank2/SmartResponseXE>