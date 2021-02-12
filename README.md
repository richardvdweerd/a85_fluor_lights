
# DCC Fluorencent Lights for model trains

Fluorencent lights simulator 

Function 4 turns lights on in random order, flickering while turning on to simulate fluorescent tube lights
Function 0 turns front / rear lights on, depending on direction
Depending on speed (<=1) and direction, the cab light (the last neopixel) is turned on

Depends on custom hardware on EasyEda:
https://easyeda.com/editor#id=8dd5c845272c4ee3b02a5e7bf703b0bd|b3c5f66cd8e042c0a457cb712b1af14f|844aaadede394cb6b4f7d844c48b101b

---

## TODO:
 - test on functions F13 - F28 (my handhelds don't support these numbers)

---

## Nice to have
 - possiblity to change decoder address on main or program track

---



## Remote programming
How to use remote programming (only the attiny 85 will be programmed, not the dcc-decoder of your locomotive):
1. stop the train on which is the attiny to be programmed to make sure there is good contact
    - **NOTE**: if you have multiple coaches on the track with attiny's with the same decoder address, they will all be programmed
2. using the function buttons (on the handheld) select the decoderaddress of the attiny('s) to be programmed 
3. type the decoderaddress by **slowly** pressing the function buttons (so press 0, 9, 4, 2 or 0, 0, 0, 7, allowing the command to be send)
4. wait for the response of the coach lights

5. slowly type the register number to change:
    - 03: decoder address (0001 - 9999)
    - 04: function number for the coach light (0 - 12)
    - 05: maximum speed to light the cabin light (0 - speed steps)
   - 06: coach color red (0 - 255)
   - 07: coach color green (0 - 255)
   - 08: coach color blue (0 - 255)
   - 09: cabin color red (0 - 255)
   - 10: cabin color green (0 - 255)
   - 11: cabin color blue (0 - 255)
6. wait for the response of the coach lights
7. slowly type the value for the chosen register
8. wait for the response of the coach lights
    - values will be effective immediately
      - color changes to the rgb-values of the coach and cabin lights are shown
      - changing the then decoder address will stop programming mode
9. repeat from step 5 or when you changed the decoder address from step 3

**NOTE:** when no function button is pressed for x seconds, the programming mode is cancelled, ignoring the last selected register


