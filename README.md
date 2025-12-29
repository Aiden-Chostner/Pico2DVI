# Pico2DVI
A couple of months ago, I began exploring circuit design, and I liked the idea of creating my own custom graphics card. After some back and forth about choosing the requirements for the GPU, I settled on using a Raspberry Pi Pico as the main processor. It already had graphics libraries made for it, so it was simple to add HDMI support. I saw [PicoDVI by Luke Wren](https://github.com/Wren6991/PicoDVI/tree/master), but I wanted to add more RAM and some extra capabilities. I added 32MB of VRAM, which is about 250 times smaller and 36 million times slower than the memory on a modern GPU. (As you can probably tell, I was not going for speed.) The microprocessor also has wifi and extra general-purpose pins. That means the card can take in user inputs like a keyboard and output data in ways other than just a screen. It can also get data from the Internet. For example, you can fetch information about the stock market. I prioritized the card being versatile instead of fast, allowing someone to do almost anything with it.

## Specs
* RP2350 Core
* 32 MB VRAM
* HDMI DVI
* Extra I/O
* Subzero Cooler
* Wifi 2.4 GHz

## Schematic
![alt text](images/schematic.png)

## PCB
![alt text](images/pcb.png)
