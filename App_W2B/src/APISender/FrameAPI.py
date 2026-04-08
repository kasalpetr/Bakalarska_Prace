import json
from datetime import datetime
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, imageConfigJsonPath


framesUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/frames"

FRAME_MARGIN = 60  # px margin around content inside the frame
FRAME_GAP = 120    # px gap between imported frames on the board


def buildHeaders(apiToken):
    return {
        "accept": "application/json",
        "content-type": "application/json",
        "authorization": f"Bearer {apiToken}",
    }


def fetchExistingFrames(apiToken): # Fetch existing frames from the Miro board to determine placement of the new frame
    frames = []
    cursor = None

    while True:
        params = {"limit": 50}
        if cursor:
            params["cursor"] = cursor

        response = requests.get(framesUrl, headers=buildHeaders(apiToken), params=params, timeout=30)
        if not response.ok:
            print(f"Failed to fetch existing frames: {response.status_code} {response.text}")
            return []

        payload = response.json()
        frames.extend(payload.get("data", []))

        cursor = payload.get("cursor") or payload.get("nextCursor")
        if not cursor:
            break

    return frames


def getFrameBounds(frame): # Helper function to get the bounds of a frame
    geometry = frame.get("geometry", {})
    position = frame.get("position", {})

    width = float(geometry.get("width", 0))
    height = float(geometry.get("height", 0))
    centerX = float(position.get("x", 0))
    centerY = float(position.get("y", 0))

    if width <= 0 or height <= 0:
        return None

    left = centerX - width / 2.0
    top = centerY - height / 2.0
    return left, top, width, height


def computeFramePosition(frameWidth, frameHeight, existingFrames): # Compute the position for the new frame based on existing frames to avoid overlap
    validBounds = []

    for frame in existingFrames: # Iterate through existing frames and collect valid bounds
        bounds = getFrameBounds(frame)
        if bounds is not None:
            validBounds.append(bounds)

    if not validBounds:
        return frameWidth / 2.0, frameHeight / 2.0

    rightMostBounds = validBounds[0]
    rightMostRight = rightMostBounds[0] + rightMostBounds[2]

    for bounds in validBounds[1:]: # Find the rightmost frame by comparing the right edges of the frames
        currentRight = bounds[0] + bounds[2]
        if currentRight > rightMostRight:
            rightMostBounds = bounds
            rightMostRight = currentRight

    rightMostLeft = rightMostBounds[0]
    rightMostTop = rightMostBounds[1]
    rightMostWidth = rightMostBounds[2]
    rightMostRight = rightMostLeft + rightMostWidth
    newLeft = rightMostRight + FRAME_GAP
    newTop = rightMostTop
    return newLeft + frameWidth / 2.0, newTop + frameHeight / 2.0 # Return the center position for the new frame based on the calculated left and top


def uploadFrame(imageSizePath=imageConfigJsonPath, apiToken=miroApiToken):
    imageSizePath = Path(imageSizePath)
    if not imageSizePath.exists():
        print(f"imageConfig.json not found at {imageSizePath}, using default 1600x1600")
        imageWidth, imageHeight = 1600.0, 1600.0
    else:
        data = json.loads(imageSizePath.read_text(encoding="utf-8"))
        imageWidth  = float(data["width"])
        imageHeight = float(data["height"])

    frameWidth  = imageWidth  + 2 * FRAME_MARGIN
    frameHeight = imageHeight + 2 * FRAME_MARGIN
    existingFrames = fetchExistingFrames(apiToken)
    centerX, centerY = computeFramePosition(frameWidth, frameHeight, existingFrames)

    payload = {
        "data": {
            "format": "custom",
            "title": datetime.now().strftime("Tabule %Y-%m-%d %H:%M:%S"),
            "type": "freeform",
            "showContent": True,
        },
        "style": {"fillColor": "#FFFFFF"},
        "position": {"x": centerX, "y": centerY},
        "geometry": {"height": frameHeight, "width": frameWidth},
    }

    response = requests.post(framesUrl, json=payload, headers=buildHeaders(apiToken), timeout=30)

    if response.ok:
        frameId = response.json().get("id")
        print(f"Uploaded frame: {response.status_code}, frameId={frameId}, size={int(frameWidth)}x{int(frameHeight)}")
        return frameId, -float(FRAME_MARGIN), -float(FRAME_MARGIN), centerX, centerY, frameWidth, frameHeight

    print(f"Failed to upload frame: {response.status_code} {response.text}")
    return None, None, None, None, None, None, None


if __name__ == "__main__":
    uploadFrame()
