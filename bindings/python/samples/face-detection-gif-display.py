#!/usr/bin/env python
import cv2
import subprocess
import signal
import sys
import os
import time
from threading import Thread, Event

# Global variables for process management
gif_process = None
face_detected = Event()
running = True

def start_gif_display(gif_file):
    """Start the gif display process"""
    try:
        # Use sudo to run the gif-viewer script
        cmd = ['sudo', 'python3', 'gif-viewer.py', gif_file]
        process = subprocess.Popen(cmd)
        return process
    except Exception as e:
        print(f"Error starting gif display: {e}")
        return None

def stop_gif_display(process):
    """Stop the gif display process"""
    if process:
        process.terminate()
        try:
            process.wait(timeout=5)  # Wait up to 5 seconds for the process to terminate
        except subprocess.TimeoutExpired:
            process.kill()  # Force kill if it doesn't terminate

def face_detection_loop():
    """Main face detection loop"""
    global gif_process, running
    
    # Initialize camera
    cap = cv2.VideoCapture(0)  # Try default camera first
    if not cap.isOpened():
        for device in [2, 3, 1]:  # Try other devices if default fails
            cap = cv2.VideoCapture(f"/dev/video{device}")
            if cap.isOpened():
                break
    
    if not cap.isOpened():
        print("Error: Could not open camera")
        sys.exit(1)

    # Load face detection classifier
    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

    face_detected_count = 0
    no_face_count = 0
    
    while running:
        ret, frame = cap.read()
        if not ret:
            continue

        # Convert frame to grayscale for face detection
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Detect faces
        faces = face_cascade.detectMultiScale(gray, 1.1, 4)
        
        if len(faces) > 0:
            face_detected_count += 1
            no_face_count = 0
            
            # Start gif display after consistent face detection
            if face_detected_count >= 5 and not face_detected.is_set():
                face_detected.set()
                if gif_process is None:
                    gif_process = start_gif_display("1.gif")
        else:
            no_face_count += 1
            face_detected_count = 0
            
            # Stop gif display after consistent no face detection
            if no_face_count >= 10 and face_detected.is_set():
                face_detected.clear()
                if gif_process:
                    stop_gif_display(gif_process)
                    gif_process = None

        # Draw rectangles around detected faces
        for (x, y, w, h) in faces:
            cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
            cv2.putText(frame, 'Face', (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 0, 0), 2)

        # Show camera feed
        cv2.imshow('Camera Feed', frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Cleanup
    cap.release()
    cv2.destroyAllWindows()
    if gif_process:
        stop_gif_display(gif_process)

def signal_handler(signum, frame):
    """Handle cleanup on system signals"""
    global running
    running = False
    if gif_process:
        stop_gif_display(gif_process)
    cv2.destroyAllWindows()
    sys.exit(0)

def main():
    # Register signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    try:
        # Start face detection in the main thread
        face_detection_loop()
    except Exception as e:
        print(f"Error in main loop: {e}")
    finally:
        # Cleanup
        if gif_process:
            stop_gif_display(gif_process)
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main() 