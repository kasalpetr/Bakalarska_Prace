import base64
import requests
import json
import sys

API_KEY = "AIzaSyBe-G8nctRHZdRkWcltqKr7cEHFen0m05c"

def test_google_vision(img_path): # Call the Google Vision API to perform text detection on the specified image and save the results to a JSON file
    with open(img_path, "rb") as f:
        img_base64 = base64.b64encode(f.read()).decode("utf-8")

    url = f"https://vision.googleapis.com/v1/images:annotate?key={API_KEY}"
    payload = {
        "requests": [{
            "image": {"content": img_base64},
            "features": [{"type": "DOCUMENT_TEXT_DETECTION"}]
        }]
    }

    response = requests.post(url, json=payload)
    
    if response.status_code != 200:
        print("Error:", response.text)
        return

    data = response.json() 
    
    with open("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/json/detectedText.json", "w") as f:
        json.dump(data, f, ensure_ascii=False, indent=2) # Save the API response to a JSON file for later use in uploading text annotations to the Miro board
    

if __name__ == "__main__":
        if len(sys.argv) > 1:
            image_path = sys.argv[1]
        else:
            raise EOFError("No image path provided.")
        test_google_vision(image_path)