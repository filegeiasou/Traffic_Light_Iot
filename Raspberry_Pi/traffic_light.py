from gpiozero import TrafficLights
from time import sleep
import requests

key = "YOUR_KEY"

def updateData(status, field):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + key + "&" + field + "=" + status)
    if (x.status_code == 200):
        print(f"Status: {status}, Field: {field}")
        print(F"Entry: "+ x.text)
    else:
        print(f"Failed to update Field {field}, Status: {status}, Response: {x.status_code}")

if __name__ == "__main__":
    # Option 1: With TrafficLights
    lights = TrafficLights(25, 8, 7)
    while True:
        # Red light on and off 
        lights.red.on(); updateData("1", "field1")
        sleep(30)
        lights.red.off(); updateData("0", "field1")
        sleep(15)  # thingspeak needs 15 secs between requests?
        
        # Green light on and off
        lights.green.on(); updateData("1", "field2")
        sleep(30)
        lights.green.off(); updateData("0", "field2")
        sleep(15)  # thingspeak needs 15 secs between requests?
        
        # Amber light on and off
        lights.amber.on(); updateData("1", "field3")
        sleep(20)
        lights.amber.off(); updateData("0", "field3")
        sleep(15)  # thingspeak needs 15 secs between requests?

# Option 2: With Three Different LEDs
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
