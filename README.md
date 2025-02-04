# robot24

## Starting point

RaspiOS
- Minimal if you don't want desktop
- For doing sutff with camera and/or OpenCV, an X desktop is probably better
- Can skip the "maximum" image which has office apps etc
Download, build, and install pigpio

## Configuring OS

`sudo raspi-config`
  - Disable serial login prompt if you want to use those GPIO pins
    - Interface Options -> Serial Port
  - set locale to en.us UTF8 (locale on first boot seems kind of random?)
  - set keyboard to en.us-generic 104

# Configure bootup program

Use example file: /etc/systemd/system/robot.service
  - `WantedBy=multi-user.target` entry is what makes it run on bootup
  - Think about CPU affinity so it doesn't get interrupted?
    - `isolcpus` in /boot/cmdline.txt
    - Pigpio doesn't seem to need it

# Vision
get openCV built?
  - https://towardsdatascience.com/installing-opencv-in-pizero-w-8e46bd42a3d3
It looks like there are openCV packages in apt
  - apt-cache search opencv
