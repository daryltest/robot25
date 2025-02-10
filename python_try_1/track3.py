from picamera2 import Picamera2
import cv2
import time

# define the lower and upper boundaries of the "target"
# color in the HSV color space, then initialize the
# list of tracked points
targetLower = (108, 190, 6)
targetUpper = (120, 255, 255)

picam2 = Picamera2()
config = picam2.create_still_configuration(main = {"size": (820, 616), "format": "RGB888"}, buffer_count = 3, queue = False)
picam2.configure(config)
picam2.set_controls({"AnalogueGain": 2.0})

picam2.start(show_preview=False)

# allow the camera or video file to warm up
time.sleep(6.0)

# keep looping
while True:
	frame = picam2.capture_array("main")

	if frame is None:
		print("frame == None")
		break
	print(f"got frame {frame.shape}")

	blurred = cv2.GaussianBlur(frame, (11, 11), 0)
	hsv = cv2.cvtColor(blurred, cv2.COLOR_RGB2HSV)

	# construct a mask for the color "green", then perform
	# a series of dilations and erosions to remove any small
	# blobs left in the mask
	mask = cv2.inRange(hsv, targetLower, targetUpper)

	masked = cv2.bitwise_and(frame, frame, mask=mask)
	
	cv2.imshow("track3", masked)

	key = cv2.waitKey(0) & 0xFF
	# if the 'q' key is pressed, stop the loop
	if key == ord("q"):
		break
	
	mask = cv2.erode(mask, None, iterations=2)
	mask = cv2.dilate(mask, None, iterations=2)

	# find contours in the mask and initialize the current
	# (x, y) center of the ball
	cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)
	cnts = cnts[0]
	center = None

	# only proceed if at least one contour was found
	if len(cnts) > 0:
		# find the largest contour in the mask, then use
		# it to compute the minimum enclosing circle and
		# centroid
		c = max(cnts, key=cv2.contourArea)
		((x, y), radius) = cv2.minEnclosingCircle(c)

		M = cv2.moments(c)
		center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

		# only proceed if the radius meets a minimum size
		if radius > 3:
			# draw the circle and centroid on the frame,
			# then update the list of tracked points
			cv2.circle(frame, (int(x), int(y)), int(radius),
				(255, 255, 0), 2)
			cv2.circle(frame, center, 5, (255, 0, 0), -1)

	# show the frame to our screen
	cv2.imshow("track3", frame)

	key = cv2.waitKey(0) & 0xFF
	# if the 'q' key is pressed, stop the loop
	if key == ord("q"):
		break

picam2.close()

cv2.destroyAllWindows()
