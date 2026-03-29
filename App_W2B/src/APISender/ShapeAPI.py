import json
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, shapesJsonPath


shapesUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/shapes"
defaultShapeSize = 100
shapeTypeMap = {
	"rectangle": "rectangle",
	"circle": "circle",
	"triangle": "triangle",
}


def buildHeaders(apiToken): # Helper function to build the headers for API
	return {
		"accept": "application/json",
		"content-type": "application/json",
		"authorization": f"Bearer {apiToken}",
	}


def getShapeCenter(shape): # Helper function to calculate the center coordinates
	width = shape.get("width", defaultShapeSize)
	height = shape.get("height", defaultShapeSize)
	centerX = shape.get("x", 0) + width / 2
	centerY = shape.get("y", 0) + height / 2
	return centerX, centerY, width, height


def uploadShapes(jsonPath=shapesJsonPath, apiToken=miroApiToken): # Main function to read detected shapes from JSON and upload them to Miro board via API
	jsonPath = Path(jsonPath)
	if not jsonPath.exists():
		print(f"Shapes JSON not found: {jsonPath}")
		return

	with jsonPath.open("r", encoding="utf-8") as file:
		shapes = json.load(file)

	headers = buildHeaders(apiToken)

	for shape in shapes:
		centerX, centerY, width, height = getShapeCenter(shape)
		miroShapeType = shapeTypeMap.get(shape.get("type", "rectangle"), "rectangle")

		payload = {
			"data": {"shape": miroShapeType},
			"position": {
				"x": centerX,
				"y": centerY,
			},
			"style": {
				"fillColor": "#ffffff",
				"fillOpacity": "0.0",
				"fontFamily": "arial",
				"fontSize": "14",
				"textAlign": "center",
				"textAlignVertical": "middle",
				"borderStyle": "normal",
				"borderColor": shape.get("colorHex", "#1a1a1a"),
				"borderOpacity": "1.0",
				"borderWidth": "2.0",
			},
			"geometry": {
				"width": width,
				"height": height,
			},
		}

		response = requests.post(shapesUrl, json=payload, headers=headers, timeout=30)

		if response.ok:
			print(f"Uploaded shape {miroShapeType}: {response.status_code}")
			continue

		print(
			f"Failed to upload shape {miroShapeType}: "
			f"{response.status_code} {response.text}"
		)


if __name__ == "__main__":
	uploadShapes()
