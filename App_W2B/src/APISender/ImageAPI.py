import json
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, imageSizeJsonPath, residualImagePath


imagesUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/images"


def buildHeaders(apiToken):
    return {
        "accept": "application/json",
        "authorization": f"Bearer {apiToken}",
    }


def uploadImage(imagePath=residualImagePath, imageSizePath=imageSizeJsonPath, apiToken=miroApiToken, frameId=None, frameOffsetX=0.0, frameOffsetY=0.0):
    imagePath = Path(imagePath)
    if not imagePath.exists():
        print(f"Residual image not found: {imagePath}")
        return

    imageSizePath = Path(imageSizePath)
    if not imageSizePath.exists():
        print(f"imageSize.json not found: {imageSizePath}")
        return

    sizeData = json.loads(imageSizePath.read_text(encoding="utf-8"))
    imageWidth = float(sizeData["width"])
    imageHeight = float(sizeData["height"])

    centerX = imageWidth / 2 - frameOffsetX
    centerY = imageHeight / 2 - frameOffsetY

    imageData = {
        "data": {},
        "position": {"x": centerX, "y": centerY},
        "geometry": {"width": imageWidth},
    }
    if frameId:
        imageData["parent"] = {"id": frameId}

    with imagePath.open("rb") as imgFile:
        response = requests.post(
            imagesUrl,
            headers=buildHeaders(apiToken),
            data={"data": json.dumps(imageData)},
            files={"resource": (imagePath.name, imgFile, "image/png")},
            timeout=60,
        )

    if response.ok:
        print(f"Uploaded residual image: {response.status_code}")
    else:
        print(f"Failed to upload residual image: {response.status_code} {response.text}")


if __name__ == "__main__":
    uploadImage()
