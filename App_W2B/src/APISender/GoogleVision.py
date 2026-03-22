import base64
import requests
import json
import sys
from pathlib import Path

from Config import googleVisionApiKey, textJsonPath


def detectTextWithGoogleVision(imagePath):
    with open(imagePath, "rb") as imageFile:
        imageBase64 = base64.b64encode(imageFile.read()).decode("utf-8")

    requestUrl = f"https://vision.googleapis.com/v1/images:annotate?key={googleVisionApiKey}"
    requestPayload = {
        "requests": [{
            "image": {"content": imageBase64},
            "features": [{"type": "DOCUMENT_TEXT_DETECTION"}]
        }]
    }

    response = requests.post(requestUrl, json=requestPayload)

    if response.status_code != 200:
        print("Error:", response.text)
        return

    responseData = response.json()

    outputPath = Path(textJsonPath)
    with outputPath.open("w", encoding="utf-8") as outputFile:
        json.dump(responseData, outputFile, ensure_ascii=False, indent=2)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        imagePath = sys.argv[1]
    else:
        raise EOFError("No image path provided.")
    detectTextWithGoogleVision(imagePath)