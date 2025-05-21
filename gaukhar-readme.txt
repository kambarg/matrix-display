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
$ sudo python3 gif-viewer.py  ../../../1.gif


4. Running face detection python script
# Create python virtual environment
$ mkdir matrix_venv
$ python3 -m venv matrix_venv

# Activate the virtual environment
$ source matrix_venv/bin/activate


4. Install face detection python library

# Create a new directory for your project (if not already created)
$ pwd
~/Desktop/gaukhar/matrix-display



# Install OpenCV
$ pip install opencv-python

# Install numpy (if not already installed with OpenCV)
$ pip install numpy

# Install the LED matrix library
cd bindings/python
pip install -e .

# Make sure you're in the virtual environment (you should see (matrix_venv) in your prompt)
# Even with venv, we still need sudo for LED matrix GPIO access
sudo -E env "PATH=$PATH" python3 face-detection-haar.py


