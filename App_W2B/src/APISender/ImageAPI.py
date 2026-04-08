import json
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, imageConfigJsonPath, residualImagePath


imagesUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/images"


def buildHeaders(apiToken):
    return {
        "accept": "application/json",
        "authorization": f"Bearer {apiToken}",
    }


def uploadImage(imagePath=residualImagePath, imageSizePath=imageConfigJsonPath, apiToken=miroApiToken, frameId=None, frameOffsetX=0.0, frameOffsetY=0.0):
    imagePath = Path(imagePath)
    if not imagePath.exists():
        print(f"Residual image not found: {imagePath}")
        return

    imageSizePath = Path(imageSizePath)
    if not imageSizePath.exists():
        print(f"imageConfig.json not found: {imageSizePath}")
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


SOURCE_GAP = 80  # px between bottom of frame and top of source image


def uploadSourceImage(configPath=imageConfigJsonPath, apiToken=miroApiToken,
                      frameCenterX=0.0, frameCenterY=0.0, frameHeight=0.0):
    configPath = Path(configPath)
    if not configPath.exists():
        print(f"imageConfig.json not found: {configPath}")
        return

    config = json.loads(configPath.read_text(encoding="utf-8"))
    imagePath = Path(config.get("path", ""))
    if not imagePath.exists():
        print(f"Source image not found: {imagePath}")
        return

    imageWidth  = float(config["width"])
    imageHeight = float(config["height"])

    sourceX = frameCenterX
    sourceY = frameCenterY + frameHeight / 2 + SOURCE_GAP + imageHeight / 2

    imageData = {
        "data": {},
        "position": {"x": sourceX, "y": sourceY},
        "geometry": {"width": imageWidth},
    }

    suffix = imagePath.suffix.lower()
    mimeType = "image/jpeg" if suffix in (".jpg", ".jpeg") else "image/png"

    with imagePath.open("rb") as imgFile:
        response = requests.post(
            imagesUrl,
            headers=buildHeaders(apiToken),
            data={"data": json.dumps(imageData)},
            files={"resource": (imagePath.name, imgFile, mimeType)},
            timeout=60,
        )

    if response.ok:
        print(f"Uploaded source image: {response.status_code}")
    else:
        print(f"Failed to upload source image: {response.status_code} {response.text}")


if __name__ == "__main__":
    uploadImage()
