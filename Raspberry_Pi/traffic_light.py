from gpiozero import LED, TrafficLights
from time import sleep

# Option 1: With Three Different LEDs
# red = LED(25)
# yellow = LED(8)
# green = LED(7)

# leds = [red, green, yellow]
# while True:
#     for led in leds:
#         led.on()
#         if (led == yellow):
#             sleep(20)
#         else:
#             sleep(30)
#         led.off()

# Option 2: With TrafficLights
lights = TrafficLights(25, 8, 7)
while True:
    lights.red.on()
    sleep(30)
    lights.red.off()
    lights.green.on()
    sleep(30)
    lights.green.off()
    lights.amber.on()
    sleep(20)
    lights.off()
