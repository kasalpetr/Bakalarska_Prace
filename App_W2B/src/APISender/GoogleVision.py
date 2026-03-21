import base64
import requests
import json

API_KEY = "AIzaSyBe-G8nctRHZdRkWcltqKr7cEHFen0m05c"
IMAGE_PATH = "/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/TextShape.jpg" 

def test_google_vision(img_path):
    with open(img_path, "rb") as f:
        img_base64 = base64.b64encode(f.read()).decode("utf-8")

    url = f"https://vision.googleapis.com/v1/images:annotate?key={API_KEY}"
    payload = {
        "requests": [{
            "image": {"content": img_base64},
            "features": [{"type": "DOCUMENT_TEXT_DETECTION"}]
        }]
    }

    print(f"Sending image {img_path} to Google Vision...")
    response = requests.post(url, json=payload)
    
    if response.status_code != 200:
        print("Error:", response.text)
        return

    data = response.json()
    
    if 'fullTextAnnotation' in data['responses'][0]:
        print("\nDetected text:")
        print(data['responses'][0]['fullTextAnnotation']['text'])
        
        print("\nCoordinates of the first block (for masking):")
        first_block = data['responses'][0]['fullTextAnnotation']['pages'][0]['blocks'][0]
        print(first_block['boundingBox']['vertices'])
    else:
        print("Text not detected.")

if __name__ == "__main__":
    test_google_vision(IMAGE_PATH)