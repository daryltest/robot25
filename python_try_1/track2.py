import time
t_0 = time.monotonic()
from picamera2 import Picamera2
import cv2

picam2 = Picamera2()
config = picam2.create_still_configuration(main= {"size": (1640, 1232)}, lores = {"size": (410, 308)}, display = "lores", buffer_count = 3, queue = False)
picam2.configure(config)

t_1 = time.monotonic()
print(f"startup {round(t1 - t0, 3)}s")
t_0 = time.monotonic()

picam2.set_controls({"ExposureTime": 10000, "AnalogueGain": 5}) #Shutter time and analogue signal boost
picam2.start(show_preview=True)

t_1 = time.monotonic()
print(f"startup {round(t1 - t0, 3)}s")

time.sleep(5)  #enjoy the preview

t_0 = time.monotonic()
img = picam2.capture_array("lores") #this takes a picture. img can be used with cv2
picam2.close() #when you're done taking photos, this closes the camera connection

t_1 = time.monotonic()
print(f"capture {round(t1 - t0, 3)}s")
t_0 = time.monotonic()

print("width height\t:", *img.shape[0:2][::-1])

cv2.imshow("Title", img) #resize the image to a quarter of the original size
# cv2.imshow("Title", cv2.resize(img, (0,0), fx=0.25, fy=0.25)) #resize the image to a quarter of the original size

cv2.waitKey(0) #Wait until a key is pressed while the img window is selected
