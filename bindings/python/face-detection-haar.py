import time
import sys
import cv2
import numpy as np
import platform
import os

def print_camera_properties(cap):
    """Print current camera properties for debugging"""
    props = [
        (cv2.CAP_PROP_FRAME_WIDTH, "Width"),
        (cv2.CAP_PROP_FRAME_HEIGHT, "Height"),
        (cv2.CAP_PROP_FPS, "FPS"),
        (cv2.CAP_PROP_FOURCC, "FOURCC"),
        (cv2.CAP_PROP_BRIGHTNESS, "Brightness"),
        (cv2.CAP_PROP_CONTRAST, "Contrast"),
        (cv2.CAP_PROP_SATURATION, "Saturation"),
        (cv2.CAP_PROP_HUE, "Hue"),
        (cv2.CAP_PROP_GAIN, "Gain"),
        (cv2.CAP_PROP_EXPOSURE, "Exposure")
    ]
    print("\nCamera Properties:")
    for prop_id, prop_name in props:
        value = cap.get(prop_id)
        print(f"{prop_name}: {value}")

# Initialize webcam with better error handling
def init_camera():
    print("\nAttempting to initialize camera...")
    
    # Try different video devices (Creality cam creates multiple devices)
    for device in [2, 3, 1, 0]:  # Try video2 first as it might be uncompressed
        device_path = f"/dev/video{device}"
        if not os.path.exists(device_path):
            continue
            
        print(f"\nTrying {device_path}...")
        cap = cv2.VideoCapture(device_path)
        if cap.isOpened():
            print(f"Successfully opened {device_path}")
            
            # Try to set a common format
            cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M','J','P','G'))
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
            cap.set(cv2.CAP_PROP_FPS, 30)
            
            # Try to read a test frame
            ret, frame = cap.read()
            if ret:
                print(f"Successfully read a test frame from {device_path}")
                print_camera_properties(cap)
                return cap
            else:
                print(f"Failed to read test frame from {device_path}")
                cap.release()
    
    print("\nError: Could not initialize camera. Debug info:")
    print(f"OpenCV version: {cv2.__version__}")
    print(f"Platform: {platform.system()}")
    print(f"Python version: {sys.version}")
    print("\nPlease ensure:")
    print("1. Your webcam is properly connected")
    print("2. You have proper permissions (try: sudo usermod -a -G video $USER)")
    print("3. Try running: v4l2-ctl --list-devices")
    sys.exit(1)

# Initialize camera
cap = init_camera()

# Load face detection classifier
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# Create named window for camera feed
cv2.namedWindow('Camera Feed', cv2.WINDOW_NORMAL)
cv2.resizeWindow('Camera Feed', 640, 480)

# Run
face_detected = False
running = True

while running:
    # clock.tick(60)

    # for event in pygame.event.get():
    #     if event.type == pygame.QUIT:
    #         running = False
    #     elif event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
    #         running = False

    # Read frame from webcam
    ret, frame = cap.read()
    if not ret:
        continue

    # Convert frame to grayscale for face detection
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    
    # Detect faces
    faces = face_cascade.detectMultiScale(gray, 1.1, 4)
    
    # # Variables for robot's reaction
    # current_face_x = None
    # current_face_detected = len(faces) > 0

    # Draw rectangles around detected faces
    for (x, y, w, h) in faces:
        # Draw rectangle around face
        cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
        # Add text label
        cv2.putText(frame, 'Face', (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 0, 0), 2)

    # Show camera feed
    cv2.imshow('Camera Feed', frame)
    cv2.waitKey(1)  # Required for CV window to update

    # # Process detected faces for robot animation
    # if current_face_detected:
    #     # Use the first detected face for eye tracking
    #     x, y, w, h = faces[0]
    #     # Calculate relative x position of face (0 to 1)
    #     current_face_x = (x + w/2) / frame.shape[1]
    #     # Keep smiling while face is detected
    #     draw_robot(smile=True, face_x=current_face_x)
    # else:
    #     # Stop smiling when face is lost
    #     draw_robot(smile=False, face_x=None)

# Cleanup
cap.release()
cv2.destroyAllWindows()
pygame.quit()
sys.exit()