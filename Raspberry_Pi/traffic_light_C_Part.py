from gpiozero import TrafficLights
from time import sleep
import requests
import threading

write_key1 = "R1LCUDBNUIP9GOW3"
read_key1 = "HKZJX0YK40A3C4X5"
write_key2 = "NNF1B9GGD4OMXA1M"
read_key2 = "6WSE17V3ZM7WYQ7R"

def updateData(redStatus, greenStatus, amberStatus):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + write_key1 + "&field1=" + greenStatus + "&field2=" + amberStatus + "&field3=" + redStatus)
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

def readField8(api_key, channel):
    x = requests.get(f"https://api.thingspeak.com/channels/"+ channel + "/feeds.json?api_key=" + api_key + "&results=2")
    if (x.status_code == 200):
        print(f"Field8: {x.json()['feeds'][1]['field8']}")
    else:
        print(f"Failed to read, Response: {x.status_code}")

# Thread1 function for field8 timer
def field8_timer():
    while True:
        sleep(60)  # Wait for 10 minutes
        updateField8("1", write_key1)
        updateField8("1", write_key2)
        sleep(20)   # Keep it on for 1 minute
        updateField8("0", write_key1)
        updateField8("0", write_key2)

def field8_checker():
    while True:
        val1 = readField8(read_key1,"2704625")
        val2 = readField8(read_key2, "2711583")
        if val1 == val2 == "1":
            lights.amber.on()
            lights.red.off()
            lights.green.off()
            print("Field8=0, Amber light:ON")
        sleep(20)  # Check every minute

if __name__ == "__main__":
    # Option 1: With TrafficLights
    lights = TrafficLights(7, 8, 25) # Red, Amber, Green
    # Setup field8
    updateField8("0", write_key1) 
    updateField8("0", write_key2)
    sleep(15)

    # Start the field8 timer in a separate thread
    thread1 = threading.Thread(target=field8_timer)
    thread1.daemon = True
    thread1.start()
    # Start the field8 checker in a separate thread
    thread2 = threading.Thread(target=field8_checker)
    thread2.daemon = True
    thread2.start()

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
