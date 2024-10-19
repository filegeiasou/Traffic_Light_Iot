from gpiozero import LED, TrafficLights
from time import sleep

# Option 1: With Three Different LEDs
# red = LED(25)
# yellow = LED(8)
# green = LED(7)

# leds = [red, yellow, green]
# while True:
#     for led in leds:
#         led.on()
#         sleep(2)
#         led.off()

# Option 2: With TrafficLights
lights = TrafficLights(25, 8, 7)
while True:
    lights.green.on()
    sleep(1)
    lights.amber.on()
    sleep(1)
    lights.red.on()
    sleep(1)
    lights.off()