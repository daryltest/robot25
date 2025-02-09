from picamera2 import Picamera2
import cv2
import time

# define the lower and upper boundaries of the "target"
# color in the HSV color space, then initialize the
# list of tracked points
targetLower = (29, 86, 6)
targetUpper = (64, 255, 255)

picam2 = Picamera2()
config = picam2.create_still_configuration(main = {"size": (820, 616)})
picam2.configure(config)
picam2.set_controls({"AnalogueGain": 2.0})

picam2.start(show_preview=True)

# allow the camera or video file to warm up
time.sleep(10.0)

# keep looping
while True:
  frame = picam2.capture_array("main")


	# show the frame to our screen
	cv2.imshow("track3", frame)

  time.sleep(0.200);

	key = cv2.waitKey(1) & 0xFF
	# if the 'q' key is pressed, stop the loop
	if key == ord("q"):
		break

picam2.close()

cv2.destroyAllWindows()
