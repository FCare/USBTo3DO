# USBTo3DO
Connecting USB controller to the 3do

## Goal of the project
Plug a set of modern controllers to an original 3DO to emulate D-Pad, FlightStick and mouse.
Still support original 3DO controllers using daisy chain.

## License
The full project is delivered under MIT License.
You can fork, build, even sell this if you wants to.

## Supported controllers
 - **D-Pad**
	 - Any HID Gamepad/joystick class device shall work with a default mapping.
	 - Xbox360 wired controllers is supported.
	 - Xbox360 wireless controller is supported using a USB wireless adapter (https://aliexpress.com/item/1005004013649915.html)
	  > As button layout is not standard, a [specific mapper can be implemented](#dedicated_mapping).
 - **Mouse**
	 - Any USB mouse with an HID descriptor might work as mouse.
 - **FlightStick**
	  > As there is no way from the HID decriptor to determine if a controler is a flightstick or a standard controler, a [specific mapper has to be implemented](#dedicated_mapping)
 - **Wheel**
	  > Even if 3DO did not get any wheel controller, USB23DO support some controler as wheel (no throttle), mostly for Need For speed game. This support requires a [specific mapper to be implemented](#dedicated_mapping)

## List of tested USB controllers
 - **D-Pad**
   - Sixaxis PS3 controller (Wired) (https://en.wikipedia.org/wiki/Sixaxis)
   - DualShock4 PS4 controller (Wired) (https://fr.wikipedia.org/wiki/DualShock)
   - Xbox 360 wirless controllers using a USB PC receiver (https://aliexpress.com/item/1005004013649915.html)
   - 8bitDo SN30 pro (https://www.8bitdo.com/sn30-pro-usb-gamepad/)
   - 8BitDo M30 controller (https://www.8bitdo.com/m30/)
   - Retrobit Saturn controller (https://retro-bit.com/sega-collaboration/#saturn2-4)
   - Classical WII controllers with USB adapter (https://www.mayflash.com/product/showproduct.php?id=21)
   - Orignial sega saturn controller with adapter (https://www.mayflash.com/product/showproduct.php?id=20)
   - Known issues on 8bitDo USB wireless adapter2 / Logitech gamepad F310
 - **Mouse**
   - a large set of USB mouse
 - **FlightStick**
   - Thrustmaster T-Flight Hotas X
   - Thrustmaster T-Flight Stick X
   - Logitech Extreme 3D Pro
 - **Wheel**
   - Thrustmaster T80
 
As of today, it does not support USB HUB. So if you are using a wired controller, you will need an adapter per wired controller. When you plug a USB adpater able to handle multiple controller (like wii adapater, saturn adapter or Xbox360 wireless receiver), then you can connect multiples controllers.

In case you installed the daisy chain connector, you can plug other USB adaptors or original controllers, Daisy chain will work.

## What is required
 - PCB as provided in this git project. (You can order to shops like JLCPCB using the gerber files provided)
 - One femelle DB9 connector (to connect to the 3do or to the previous controller)
 - One male DB9 (optional) to continue the controller daisy chain
 - One Waveshare rp2040 zero (https://www.waveshare.com/rp2040-zero.htm)
 - a 4 way level shihter (https://aliexpress.com/item/32771873030.html)
 - a 4.7uF - 10v capacitor
 - a DB9 cable extension (https://aliexpress.com/item/4000095438635.html)

Here is a picture of the v1.1 PCB:  

<img src="/PCB/Photos/PCB v1_1.jpg" alt="PCB V1.1" title="PCB v1.1"  width="50%">

## <a name="dedicated_mapping"></a>How to add a dedicated mapping
In order to add a specifi mapping, you have to edit


## How to compile
 - Tested on UBUNTU 20.04

 `git clone https://github.com/FCare/USBTo3DO.git`  
 `cd USBTo3DO`  
 `git submodule update --init --recursive`  
 `mkdir build;cd build`  
 `cmake ../`  
 `make -j16`  
 
 then flash RP204 with the built uf2 file.
 
 
