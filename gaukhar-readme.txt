# Check the you are in the right directory
$ pwd
/home/roamio42/Desktop/gaukhar/matrix_display
# Clean and build the project
$ sudo make clean
$ sudo  make

# Change directory to run examples
$ cd examples-api-use

# The very first example (from sources)
$ sudo ./demo -D 9 --led-rows=64 --led-cols=64

# Example with gif file displaying blue eyes aka Eve (from sources)
$ sudo ./image-example --led-rows=64 --led-cols=64 ../1.gif

# Example with blue eyes puctored pixel by pixel (new)
$ $ sudo ./eyes-example --led-rows=64 --led-cols=64

# Example with face detection and matrix display

# Create a new directory for your project (if not already created)
mkdir matrix_venv

# Create virtual environment
python3 -m venv matrix_venv

# Activate the virtual environment
source matrix_venv/bin/activate

# Install OpenCV
pip install opencv-python

# Install numpy (if not already installed with OpenCV)
pip install numpy

# Install the LED matrix library
cd bindings/python
pip install -e .

# Make sure you're in the virtual environment (you should see (matrix_venv) in your prompt)
# Even with venv, we still need sudo for LED matrix GPIO access
sudo -E env "PATH=$PATH" python3 face-detection-haar.py
