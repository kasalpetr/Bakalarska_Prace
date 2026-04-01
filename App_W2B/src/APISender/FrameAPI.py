import json
from datetime import datetime
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, imageSizeJsonPath


framesUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/frames"

FRAME_MARGIN = 60  # px margin around content inside the frame


def buildHeaders(apiToken):
    return {
        "accept": "application/json",
        "content-type": "application/json",
        "authorization": f"Bearer {apiToken}",
    }


def uploadFrame(imageSizePath=imageSizeJsonPath, apiToken=miroApiToken):
    imageSizePath = Path(imageSizePath)
    if not imageSizePath.exists():
        print(f"imageSize.json not found at {imageSizePath}, using default 1600x1600")
        imageWidth, imageHeight = 1600.0, 1600.0
    else:
        data = json.loads(imageSizePath.read_text(encoding="utf-8"))
        imageWidth  = float(data["width"])
        imageHeight = float(data["height"])

    frameWidth  = imageWidth  + 2 * FRAME_MARGIN
    frameHeight = imageHeight + 2 * FRAME_MARGIN
    centerX = frameWidth / 2.0
    centerY = frameHeight / 2.0

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
        # topLeft = -FRAME_MARGIN so ShapeAPI/TextAPI subtract it → objects shift +margin inside frame
        return frameId, -float(FRAME_MARGIN), -float(FRAME_MARGIN)

    print(f"Failed to upload frame: {response.status_code} {response.text}")
    return None, None, None


if __name__ == "__main__":
    uploadFrame()
