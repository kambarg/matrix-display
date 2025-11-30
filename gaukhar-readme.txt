==Quick start demo==
$ cd /home/roamio42/Desktop/gaukhar/matrix-display/
$ make -C examples-api-use
$ cd examples-api-use
$ sudo ./demo -D 9 --led-rows=64 --led-cols=128 --led-slowdown-gpio=4  --led-no-hardware-pulse
$ sudo ./welcome-message --led-rows=64 --led-cols=128 --led-slowdown-gpio=4  --led-no-hardware-pulse

$ make image-example
$ sudo ./image-example --led-rows=64 --led-cols=128 --led-slowdown-gpio=4  --led-no-hardware-pulse ../1.gif

=====================


1. Build the project
$ pwd
/home/roamio42/Desktop/gaukhar/matrix_display

# Clean and build the project
$ sudo make clean
$ sudo  make


2. Run existing examples
$ cd examples-api-use
$ sudo make

# The very first example (from sources)
$ sudo ./demo -D 9 --led-rows=64 --led-cols=64

# Example with blue eyes puctored pixel by pixel (new .c sources)
$ sudo ./eyes-example --led-rows=64 --led-cols=64

# Example with gif file displaying blue eyes aka Eve (from .cc sources)
$ sudo apt-get update
$ sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
$ make image-example
$ sudo ./image-example --led-rows=64 --led-cols=64 ../1.gif

# Exit exaples subdirectory
$ cd ../


3. Run python bindings examples
# Install python
$ cd bindings/python/
$ sudo apt-get update && sudo apt-get install python3-dev cython3 -y
$ make build-python 
$ sudo make install-python 

# Run python examples:
$ cd samples
$ sudo python3 gif-viewer.py  1.gif


4. Running face detection python script
# Create python virtual environment
$ mkdir matrix_venv
$ python3 -m venv matrix_venv

# Activate the virtual environment
$ source matrix_venv/bin/activate

# Install opencv package to control usb camera and for haar cascades face detection method
$ pip install opencv-python

# Run python script to open usb camera window
$ python3 face-detection-haar.py

# Run script that controls camra, detects face and displays 
gif picture to the led matrix display when face is detected:
$ python3 face-detection-gif-display.py


5. Troubleshooting Video Camera
# Check that USB camera is recognized as device in the system:
$ ls -l /dev/video* && v4l2-ctl --list-devices
# Your camera should be listed as one of the registered devices /dev/video0, /dev/video1 etc.

# Than check that camera works:
$ ffplay /dev/video0

#Verify the camera permisions:
$ groups $USER | grep video





