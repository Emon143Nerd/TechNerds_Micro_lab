from picamera import PiCamera
from picamera.array import PiRGBArray
import time
import requests
from PIL import Image
from gtts import gTTS
import os
import subprocess  # For more control over system calls

# Your OCR.Space API key
api_key = 'Use your own key,Im not sharing my private key, see readme file to know how to do it'

# Function to perform OCR on an image
def ocr_space_file(filename, overlay=False, api_key=api_key):
    """ OCR.space API request with image file. """
    with open(filename, 'rb') as f:
        r = requests.post(
            'https://api.ocr.space/parse/image',
            files={filename: f},
            data={'apikey': api_key, 'isOverlayRequired': overlay}
        )
    return r.json()

# Read from text file,(text file for easy data retrieve)
def read_medicines(file_path):
    medicines = {}
    with open(file_path, 'r') as file:
        for line in file:
            name, description, time = line.strip().split('|')
            medicines[name] = {'description': description, 'time': time}
    return medicines

# Using gtts (gtts = google text to speech)
def speak(text):
    tts = gTTS(text=text, lang='en')  # currently it's only working with Eng, other languages can be added
    tts_file = "output.mp3"
    tts.save(tts_file)
    
    # Play the audio using mpg321 ,( you have to install it on your raspberry pi in order to continue )
    try:
        subprocess.run(["mpg321", tts_file], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Failed to play audio: {e}")

# Initialize the camera (I'm using the picamera here, fast & easy, who's gonna wait for it? none)
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=(640, 480))

# Allow the camera to warm up 
time.sleep(2) # you can reduce it to 1 sec if you want

# Capture a single frame from the camera
camera.capture(rawCapture, format="bgr")
image = rawCapture.array 

# Save the image to a file
image_file = 'captured_image.jpg'
Image.fromarray(image).save(image_file)

# Perform OCR on the captured image
result = ocr_space_file(image_file)

# Print the extracted text
extracted_text = result.get('ParsedResults', [{}])[0].get('ParsedText', '')

# If no text is extracted, say "Try again"
if not extracted_text.strip():
    print("No text extracted. Try again.")
    speak("Try again.")
else:
    # Read medicines from the text file
    medicines = read_medicines('medicines.txt')

    # Flag to check if the extracted text matches a medicine
    medicine_found = False

    # Check for matches in the extracted text
    for medicine_name, details in medicines.items():
        if medicine_name.lower() in extracted_text.lower():
            output_text = f"Medicine: {medicine_name}. Description: {details['description']}. Time to take: {details['time']}."
            print(output_text)
            speak(output_text)  # Speak the output details
            medicine_found = True
            break

    # If no medicine matches, print "Not prescribed"
    if not medicine_found:
        print("Not prescribed.")
        speak("Not prescribed.")

# Cleanup
camera.close()

