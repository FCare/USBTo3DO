# USBTo3DO
Connecting USB controler to the 3do

## Goal of the project
My first goal was to plus my 8bitDo M30 controler to the 3do in order to replace the original controler.  
As 8bitDo is a xbox360 like controler, it is using a USB vendor class.
I found that RP2040 is able to handle a usb host stack using tinyUSB project.  
As it has two cores, one can be used to handle the USB while the other one is taking care of PIOs.
I got main issue about vendor class which is not well handled yet on tinyUSB. So I forked tinyUSB and I fixed the vendor class to be able to interact with xbox360 like controllers.  

On tinyUSB, I found an example about interfacing PS4 controler with HID class. Even if I do not have PS4 controller, I implemented some HID controllers I had at home. But unfortunaltely, I have no PS4 controllers, so this one is not yet supported.

## Supported controllers
 - 8BitDo M30 controller (https://www.8bitdo.com/m30/)
 - Xbox 360 wirless controllers using a USB PC receiver (https://aliexpress.com/item/1005004013649915.html)
 - Retrobit Saturn controller (https://retro-bit.com/sega-collaboration/#saturn2-4)
 - Classical WII controlers with USB adapter (https://www.mayflash.com/product/showproduct.php?id=21)
 - Orignial sega saturn controller with adapter (https://www.mayflash.com/product/showproduct.php?id=20)
 - Cheap Saturn like USB controllers (https://fr.aliexpress.com/item/32810789698.html)

As of today, it does not support USB HUB. So if you are using a wired controller, you will need an adapter per wired controller. When you plug a USB adpater able to handle multiple controller (like wii adapater, saturn adapter or Xbox360 wireless receiver), then you can connect multiples controllers.

In case you installed the daisy chain connector, you can plug other USB adaptors or original controlers, Daisy chain will work.

## What is required
 - PCB as provided in this git project. (You can order to shops like JLCPCB using the gerber files provided)
 - One femelle DB9 connector (to connect to the 3do or to the previous controler)
 - One male DB9 (optional) to continue the controler daisy chain
 - One Waveshare rp2040 zero (https://www.waveshare.com/rp2040-zero.htm)
 - a 4 way level shihter (https://aliexpress.com/item/32771873030.html)
 - a 4.7uF - 10v capacitor
 - a DB9 cable extension (https://aliexpress.com/item/4000095438635.html)

Here is a picture of the v1.0 PCB:  

<img src="/PCB/Photos/PCB v1.0.jpg" alt="PCB V1.0" title="PCB v1.0"  width="50%">

## How to compile
 - Tested on UBUNTU 20.04

 `git clone https://github.com/FCare/USBTo3DO.git`  
 `cd USBTo3DO`  
 `git submodule update --init --recursive`  
 `mkdir build;cd build`  
 `cmake ../`  
 `make -j16`  
 
 then flash RP204 with the built uf2 file.
 
 
