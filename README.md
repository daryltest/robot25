# robot25

```
g++ -Wall *.cpp -lpigpio -o robot
```

# TODO

Finish the vision/python thing

Go headless

See how it does on full-length and batteries

Startup mode based on power not ms

target power?

# 9-DOF gyro

**config.txt**
```
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
```

`sudo raspi-config` > Interface > Serial Port > login disabled / serial enabled

Serial mode setup:
```
stty -F /dev/ttyS0 speed 115200 cs8 sane
```

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

# Audio

https://forums.adafruit.com/viewtopic.php?p=1045498&sid=23159f370e2f22a3435e66a3f058726b#p1045498

**/boot/firmware/config.txt**
```
dtparam=audio=on
gpio=12,13,a5
audio_pwm_mode=2
dtoverlay=audremap,pins_12_13
```

```
aplay -l
aplay /usr/share/sounds/alsa/Front_Right.wav
speaker-test
```

# Clone

```
git clone https://github.com/daryltest/robot25.git
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
  - Still have to enable it though: `systemctl enable robot`
  - Also `status`, `start`, `stop`

  - Think about CPU affinity so it doesn't get interrupted?
    - `isolcpus=3` in /boot/firmware/cmdline.txt
    - `cat /sys/devices/system/cpu/isolated`
    - sudo taskset -c 3 ./robot
    - Pigpio doesn't seem to need it

# Vision
get openCV built?
  - https://towardsdatascience.com/installing-opencv-in-pizero-w-8e46bd42a3d3
It looks like there are openCV packages in apt
  - apt-cache search opencv

# Booting headless

https://forums.raspberrypi.com/viewtopic.php?t=344067

- sudo raspi-config system options -> boot/auto login
- make sure SSH is intalled / working
- Might need to configure VNC virtual frame buffer?
- Make sure all preview windows are off in OpenCV / libcamera?

# Dev setup

In VS Code, hit F1 and type SSH. Add ssh connection to Pi. Clone the repo on the Pi and edit in VS Code.

```
g++ -Wall *.cpp -lpigpio -o robot
```
