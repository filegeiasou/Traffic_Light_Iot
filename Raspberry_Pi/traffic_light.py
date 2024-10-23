from gpiozero import TrafficLights
from time import sleep
import requests

key1 = "R1LCUDBNUIP9GOW3"
key2 = "NNF1B9GGD4OMXA1M"

def updateData(redStatus, greenStatus, amberStatus):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + key1 + "&field1=" + greenStatus + "&field2=" + amberStatus + "&field3=" + redStatus)
    if (x.status_code == 200):
        if (redStatus == "1"):
            print("Red Light: On")
        elif (greenStatus == "1"):
            print("Green Light: On")
        elif (amberStatus == "1"):
            print("Amber Light: On")
        print(F"Entry: "+ x.text)
    else:
        print(f"Failed to update, Response: {x.status_code}")

def updateField8(status, api_key):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + api_key + "&field8=" + status)
    if (x.status_code == 200):
        print(f"Field8: {status}")
        print(F"Entry: "+ x.text)
    else:
        print(f"Failed to update, Response: {x.status_code}")

if __name__ == "__main__":
    # Option 1: With TrafficLights
    lights = TrafficLights(7, 8, 25) # Red, Amber, Green
    updateField8("0", key1)
    updateField8("0", key2)
    sleep(15)
    while True:
        # Red light on and off
        lights.red.on()
        updateData("1", "0", "0")
        sleep(30)
        
        # Green light on and off
        lights.red.off()
        lights.green.on()
        updateData("0", "1", "0")
        sleep(30)
        
        # Amber light on and off
        lights.green.off()
        lights.amber.on()
        updateData("0", "0", "1")
        sleep(20)
        lights.amber.off()

# Option 2: With Three Different LEDs
# red = LED(7)
# yellow = LED(8)
# green = LED(25)

# leds = [red, green, yellow]
# while True:
#     for led in leds:
#         led.on()
#         if (led == yellow):
#             sleep(20)
#         else:
#             sleep(30)
#         led.off()
