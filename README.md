# fish
Fish Code for wirelessly controlled servo fish, a keypad, and LCDs

The LCD modules and Keypad are connected to the internet and pull the time/date from an free api (pls dont steal my key lol)
Each code is 6 digits, and is randomly generated based on a seed from the hour. Although it uses the random function, arduino random numbers and predictable if you know the seed, and so while the LCDs and Keypad dont communicate, they both use the same seed which is set every hour, and so they both have an list of valid codes, around 840 per hour assuming no duplicates. That means there is around an 0.09% chance somone can guess a code at random, although this can be reduced by allowing longer codes or allowing less valid codes per hour, all options in the LCD and Keypad sketch. Because the LCDs are independant of the Keypad/Keypads, there can be multiple LCDs and multiple keypads, however for simplest operation only one keypad should be used. In addition, becase the valid code consensus requires no communication between the LCDs and Keypad (only a ping to find the datetime), they can be placed super far away and still work if used in the same hour. 

Limitations/Bugs im going to fix:

I have the equipment i need to test this system on order, so I havent tested it yet besides ensuring it compiles (even then arduino IDE may need to be used for a few over platform IO)

There is some weird 2D array logic that should work, but untested.
Im unsure if the ESP32 can be in STA mode and send ESP_NOW packets, again waiting to test.
Codes are currently depenant on the hour, meaning they will be the same day to day in that hour, for example, valid codes will the same every day from 8am to 9am, etc. This will be fixed later by using the hour concatenated with the date string. 

I really just made this for fun as a coding challenge, but I hope it might have real uses!
