from gpiozero import TrafficLights
from time import sleep
import requests
import threading
import time

write_key1 = ""
read_key1 = ""
write_key2 = ""
read_key2 = ""
alert = False

def updateData(redStatus, greenStatus, amberStatus):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + write_key1 + "&field1=" + greenStatus + "&field2=" + amberStatus + "&field3=" + redStatus)
    if (x.status_code == 200):
        print("---Update LED---")
        if (redStatus == "1"):
            print("Red Light: On Entry: "+ x.text)
        elif (greenStatus == "1"):
            print("Green Light: On Entry: "+ x.text)
        elif (amberStatus == "1"):
            print("Amber Light: On Entry: "+ x.text)
    else:
        print(f"Failed to update, Response: {x.status_code}")

def updateAlert(status, redStatus, greenStatus, amberStatus, api_key):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + api_key + "&field8=" + status + "&field1=" + greenStatus + "&field2=" + amberStatus + "&field3=" + redStatus)
    if (x.status_code == 200):
        print(f"Field8: " + status + " Entry: "+ x.text)
    else:
        print(f"Failed to update, Response: {x.status_code}")

def updateField8(status, api_key):
    x = requests.get(f"https://api.thingspeak.com/update?api_key=" + api_key + "&field8=" + status)
    if (x.status_code == 200):
        print(f"Field8: " + status + " Entry: "+ x.text)
    else:
        print(f"Failed to update, Response: {x.status_code}")

def readField8(api_key, channel):
    x = requests.get(f"https://api.thingspeak.com/channels/" + channel +"/fields/8.json?api_key=" + api_key +"&results=1")
    if (x.status_code == 200):
        print(f"----Field8 Read----")
        feeds = x.json().get('feeds', [])
        if feeds:
            status = feeds[0].get('field8')
            print(f"Field8: {status}")
            return status
        else:
            print("Failed to read, No feeds")
            return None
    else:
        print(f"Failed to read, Response: {x.status_code}")
        return None

# Thread1 function for field8 timer
def field8_timer():
    while True:
        sleep(160)  # Wait for 10 minutes

        print("----Field8 channel 1----")
        updateAlert("1","0", "0", "1", write_key1)
        print("----Field8 channel 2----")
        updateField8("1", write_key2)
        sleep(30)   # Keep it on for 1 minute

        print("----Field8 channel 1----")
        updateField8("0", write_key1)
        print("----Field8 channel 1----")
        updateField8("0", write_key2)

def field8_checker():
    global alert
    while True:
        sleep(80)  # Check every 30 seconds
        val1 = readField8(read_key2,"2727421")
        val2 = readField8(read_key1, "2727420")

        print("---Field8 Checker---")
        if val1 == None:
            val1 = val2
        elif val2 == None:
            val2 = val1
        elif val1 == None and val2 == None:
            continue
        print(f"val1: {val1}, val2: {val2}")
        # Check if both field8 are 1, then turn on amber light
        if val1 == val2 == "1":
            alert = True
            lights.amber.on()
            lights.red.off()
            lights.green.off()
            updateData("0", "0", "1")
            print("---Alert Variable ON---")
            print("Field8=1, Amber light:ON")
        if val1 == val2 == "0":
            alert = False
            

if __name__ == "__main__":
    # Traffic light setup
    lights = TrafficLights(7, 8, 25) # Red, Amber, Green
    # Setup field8
    print("--Field8 setup for both channels--")
    updateField8("0", write_key1) 
    updateField8("0", write_key2)
    sleep(15) # Wait for 15 seconds (Thingsopeak API limit)

    # Start the field8 timer in a separate thread
    thread1 = threading.Thread(target=field8_timer); thread1.daemon = True
    thread1.start()
    # Start the field8 checker in a separate thread
    thread2 = threading.Thread(target=field8_checker); thread2.daemon = True
    thread2.start()

    while True: 
        if alert == True:
            continue

        # Red light on
        if alert == False:
            # Delay 15 seconds before starting the cycle if alert has recently turned off
            time.sleep(15)
            
            lights.red.on()
            updateData("1", "0", "0")
            sleep(30)
        
        # Green light
        if alert == False:
            lights.red.off()
            lights.green.on()
            updateData("0", "1", "0")
            sleep(30)
        
        # Amber light
        if alert == False:
            lights.green.off()
            lights.amber.on()
            updateData("0", "0", "1")
            sleep(20)
            lights.amber.off()
