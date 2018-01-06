# Nerfgun Controller Mod
Attiny45/85 firmware to control a laserpointer and high performance LED for a nerfgun

## nerfgun-controller
I modded a laser pointer module and a bright LED on my nerfgun, added two pushbuttons to the grip and screwed a battery box to the side. Per default the battery box can hold 3 AA batteries. I removed one spring holder and reduced the capacity to two AA batteries which gave me space for a small circuit board containing a Atmel Attiny45, some resistors and two PN2222 NPN transistors to control the LED and laser (the Attiny can provide only about 10mA per output pin which is not enough to power my equipment but it is more than sufficient to switch the NPN transistors).

This little program reads inputs from the two pushbuttons, debounces them and controls the laser (one push = on, next push = off and so on) and the LED (one push = on, next push = random strobe mode, next push = off). When laser and LED are off, the program configures pinchange-interrupts for the two push buttons and put the microcontroller to sleep. You can wake it up by pushing a button (which will activate LED or laser, whatever you've pushed). My battery box has an on/off switch but I keep forgetting to switch it off. While the microcontroller is running and waiting for a button push, it pulls about 4mA continously until the battery runs dry. When it puts itself to sleep waiting for the next button push, it pulls below 100uA (which is the smallest current my multimeter can measure).

This is the circuit I used for this (-TODO:Draw circuit diagram and insert it here-)

The program as it is now will not run on a Attiny25. It compiles to a binary containing 2092 bytes, the Attiny25 can store only 2048 bytes. I'm pretty sure you can optimize the code some more to free the necessary 44 bytes, but if it has to be quick and dirty, remove the sleep mode code and the includes for interrupt.h and sleep.h. That should reduce the code enough (although it means that your circuit will always draw power for the non-sleeping Attiny).

## nerfgun-controller-3button
This is the 3 Button version of the controller. In the original design, I left one IO port unused and a friend of mine asked to add another button. One button switches the laser on and off while another button switches the LED on and off. The third button is a tactical button. If you push it and hold it pressed the following happens until you let go:
- If the LED is currently off, then the LED will go into strobe mode.
- If the LED is currently on, then it will go off.

The idea behind this is to make strobe mode quickly accessible (when you don't use the LED as flashlight) and have a quick way to temporarely switch off the LED when approaching a corner for example. Downside is that when you use the LED as flashlight, you cannot go directly into strobe mode but have to switch it off first. I think this is no real problem since you can also use the flashlight mode to blind your opponent or manually strobe the LED mashing the tactic button, but this has to be verified in battle first.

This is the circuit I used for this ![alt text](Nerfgun%20Controller%203-Button_Schaltplan.png)

Like the original two-button solution, you need at least an Attiny45 for this, too. But since the push button handling for Light and Laser are simpler here, you can downsize it by removing the btnFlash object in setup() and removing the two IF blocks using it in its conditions. This will effectively take away the tactical button ability (and the strobe mode) but the resulting binary will be small enough to run even on an Attiny25 and you will keep the sleep mode which significantly lowers the drain on your battery.

## General Troubleshooting
The strobe mode is kinda noisy. If you ever notice that your circuit behaves unusual, it could be that the strobe mode switching the LED off drops enough interference to Vcc to send your Attiny into reset state. In my case Strobe seemed inaccessible and also turned off the Laser (due to the reset). If that happens you can fix this by adding a small capacitor between the reset pin and ground. In my case 10nF was not enough but 16nF did the trick.

## Thanks to
A big thank you to Thomas Ouellet Fredericks for his Bounce2 library (https://github.com/thomasfredericks/Bounce2) which I use in this project. That one is a great piece of software I use in almost all of my projects.

Also thank you to my amazing friends who made me buying Nerf guns so that I can shoot them with a whole crapload of foam darts. :)

Finally thank you to my son who made me remember that I received basic electronics knowlede two decades ago at school and made me experimenting with it to gather more knowledge. He did so by just being born into this world so that years from now, when he is older, I can build awesome things together with him.
