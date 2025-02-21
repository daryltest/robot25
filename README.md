# robot24

## Starting point

RaspiOS
- Minimal if you don't want desktop
- For doing sutff with camera and/or OpenCV, an X desktop is probably better
- Can skip the "maximum" image which has office apps etc
Download, build, and install pigpio

## Configuring OS

`sudo raspi-config`
  - Interface Options -> Serial Port
    - Disable serial login prompt if you want to use those GPIO pins
  - Localisation Options
    - Locale: en.us UTF8 (locale on first boot seems kind of random?)
    - Keyboard: en.us-generic 104 (Generic 104, English (US), No AltGr, No Compose)

Think about making swap space bigger in `/etc/dphys-swapfile`:
```
CONF_SWAPFILE=2048
```

# Install Python OpenCV

```
python --version
sudo apt install python3-opencv
py
>>> import cv2
>>> cv2.__version__
>>> import numpy
>>> numpy.__version__
```

~~??? pip install --user imutils --break-system-packages~~

# Pigpio

https://abyz.me.uk/rpi/pigpio/download.html
https://github.com/joan2937/pigpio (also personal fork)

```
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
# (if the Python part of the install fails it may be because you need the setup tools):
sudo apt install python3-setuptools
```

# Clone

```
git clone https://github.com/daryltest/robot24.git
git config --add credential.helper "cache --timeout 86400"
git pull
git status
```

# System files

Checked in under `system-files`.

Copy wifi connection files
  - chmod 600
  - Fill in WPA password

## Configure bootup program

/etc/systemd/system/robot.service
  - `WantedBy=multi-user.target` entry is what makes it run on bootup
  - Think about CPU affinity so it doesn't get interrupted?
    - `isolcpus` in /boot/cmdline.txt
    - Pigpio doesn't seem to need it

# Vision
get openCV built?
  - https://towardsdatascience.com/installing-opencv-in-pizero-w-8e46bd42a3d3
It looks like there are openCV packages in apt
  - apt-cache search opencv
