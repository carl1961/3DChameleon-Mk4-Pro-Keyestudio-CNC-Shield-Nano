# 3DChameleon-Mk4-Pro-Keyestudio-CNC-Shield-Nano
3DChameleon Mk4 Pro   

Controller using Keyestudio Nano CNC Shield V4 (Ks0152)  https://wiki.keyestudio.com/Ks0152_keyestudio_CNC_Shield_V4

![WIN_20250828_07_41_26_Pro](https://github.com/user-attachments/assets/d5621e20-1e5d-4a01-a788-ed3ba54b2ff8)

<img width="1520" height="838" alt="3DChameleon_Mk4_Pro_Keyestudio_CNC_Shield_Nano v17" src="https://github.com/user-attachments/assets/fe26286d-725e-4c72-9f1c-521a9e77edd3" />

<img width="1520" height="838" alt="3DChameleon_Mk4_Pro_Keyestudio_CNC_Shield_Nano-2" src="https://github.com/user-attachments/assets/82795fdb-8d70-4685-a96d-91161c63aa1b" />

## endstop micro switch 

Used for button on controller, in photo connected to Cool EN , later it will be moved to like X+ and external enstop switch on  X- those both share same D9 pin.

![WIN_20250828_07_53_02_Pro](https://github.com/user-attachments/assets/f46b8b94-1fb1-4f26-a479-2d9fa4041f9e)

![WIN_20250828_07_51_41_Pro](https://github.com/user-attachments/assets/147da3c8-2bca-40e5-84fd-836c8cea4093)


https://www.3dchameleon.com/old-forum/support/help-on-setup-for-cnc-shield-v4-ankermake-m5c

AMS Multi color for any 3d printer

https://github.com/3DChameleon/3DChameleonMk4

https://www.3dchameleon.com/

https://www.youtube.com/@3d_chameleon

Easy 3D Printable 4Y splitter    https://www.printables.com/model/808136-4-in-1-ptfe-adapter

Cutter Servo Board plugged into Z drive location 12V source,  stepdown to 6 Volts for servo using DD4OAJSA (adjustable ) 

!! Do voltage adjustment before plugging servo in.!!  https://alexnld.com/product/3pcs-dd40ajsa-5-40v-to-24v-12v-5v-wide-voltage-adjustable-step-down-dc-voltage-converter-power-regulator-module/

![WIN_20250828_14_45_58_Pro](https://github.com/user-attachments/assets/71a0a95f-f97f-430c-86be-b0613b799a59)

#Things to know

[Cotestant custom work !!](https://github.com/cotestatnt/3DChameleonMk4)

cotestatnt

I successfully built and installed a slightly modified version of the 3D Chameleon color changer on a Bambulab A1, and it works really well.
I can complete a color change in less than 20 seconds because the filament path on the A1 is very short. There is still room to improve this further, for example, by increasing the stepper motor speed.

The only aspect that didn’t fully convince me is the small teeth system for securing the PTFE tubes. So, I adapted your design to accommodate these fittings that I had available, cut in half:

<img width="424" height="247" alt="image" src="https://github.com/user-attachments/assets/341ad901-7bfa-444d-9f3e-1b7733f9a439" />


I believe there is also enough space to screw in a standard PC4-M6 fitting without any major issues.

For the electronics, I repurposed some components I had on hand, using a CNC shield for an Arduino Nano and a couple of Hall sensors instead of microswitches. I also added a sensor for homing the selector without bringing the motor to a mechanical stop, as I found it to be somewhat inaccurate—the motor tends to "bounce" when it starts skipping steps. While this isn’t a major issue, since I had the sensor available, I preferred this approach.
As for the firmware, since speed is more important to me than flexibility or the max number of possible colors, I simplified the color selection process, as I usually only need 2–3 colors for my designs.









## Jumpers and pins

<img width="1323" height="840" alt="KS0152_shield" src="https://github.com/user-attachments/assets/a3f78cea-25e2-4fb8-8bb9-cd44b8ca89c4" />

