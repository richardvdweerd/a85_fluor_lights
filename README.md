
**!! WORK IN PROGRESS !!**

# DCC Fluorencent Lights for model trains

Fluorencent lights simulator 

Function 3 turns lights on in random order, flashing while turning on to simulate fluorescent tube lights
Function 4 turns lights on in the cabin, it auto turns off when the train is driving at certain speed
Function 0 turns front / rear lights on, depending on direction
Depending on speed (<=1) and direction, the cab light (the last neopixel) is turned on

Function numbers for coach and cabin lights can be defined in the header file 'trainsettings.h'.

Depends on custom hardware on EasyEda:
https://easyeda.com/editor#id=8dd5c845272c4ee3b02a5e7bf703b0bd|b3c5f66cd8e042c0a457cb712b1af14f|844aaadede394cb6b4f7d844c48b101b

---

## TODO:
 - test on functions F13 - F28 (my handhelds don't support these numbers)

## Nice to have
 - possibility to remotely change decoder address on main or program track
 - possibility to remotely change light/color of the neopixels
 - possibility to remotely change function numbers for coach and cabin lighting
 - possibitity to remotely change the max speed for cabin light to light

An attempt to implement above was started in another branch 'remote-programming'.
After implementing the code for entering the programming mode, the attempt was shut down because of memory lack on the attiny85. I wil try it again when the new micro processor (ATtiny1616) has arrived. This processor had twice the (flash) memory.
     
---

 #dcc #dcc++ #at85 #at1616 #neopixel #model-railroad #fluorenscent-light-simulator #megaTinyCore