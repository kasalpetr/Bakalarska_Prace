import json
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, textJsonPath


textsUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/texts"
defaultTextWidth = 120
minFontSize = 10
maxFontSize = 72


def buildHeaders(apiToken):
	return {
		"accept": "application/json",
		"content-type": "application/json",
		"authorization": f"Bearer {apiToken}",
	}


def extractBounds(vertices):
	xValues = [vertex.get("x", 0) for vertex in vertices]
	yValues = [vertex.get("y", 0) for vertex in vertices]

	minX = min(xValues, default=0)
	maxX = max(xValues, default=minX)
	minY = min(yValues, default=0)
	maxY = max(yValues, default=minY)

	return minX, minY, maxX, maxY


def readTextAnnotations(jsonPath):
	with Path(jsonPath).open("r", encoding="utf-8") as file:
		payload = json.load(file)

	responses = payload.get("responses", [])
	if not responses:
		return []

	annotations = responses[0].get("textAnnotations", [])
	return annotations[1:] if len(annotations) > 1 else []


def calculateFontSize(textContent, textWidth, textHeight):
	charCount = max(len(textContent), 1)
	fontByHeight = max(minFontSize, min(maxFontSize, int(round(textHeight * 0.9))))
	fontByWidth = max(minFontSize, min(maxFontSize, int(round((textWidth / charCount) * 1.65))))
	return min(fontByHeight, fontByWidth)


def uploadTexts(jsonPath=textJsonPath, apiToken=miroApiToken):
	jsonPath = Path(jsonPath)
	if not jsonPath.exists():
		print(f"Text JSON not found: {jsonPath}")
		return

	annotations = readTextAnnotations(jsonPath)
	if not annotations:
		print("No text annotations found for upload.")
		return

	headers = buildHeaders(apiToken)

	for annotation in annotations:
		textContent = annotation.get("description", "").strip()
		if not textContent:
			continue

		vertices = annotation.get("boundingPoly", {}).get("vertices", [])
		minX, minY, maxX, maxY = extractBounds(vertices)
		textWidth = max(defaultTextWidth, maxX - minX)
		textHeight = max(1, maxY - minY)
		centerX = minX + textWidth / 2
		centerY = minY + textHeight / 2
		fontSize = calculateFontSize(textContent, textWidth, textHeight)

		payload = {
			"data": {
				"content": textContent,
			},
			"style": {
				"fontSize": str(fontSize),
            },
			"position": {
				"x": centerX,
				"y": centerY,
			},
			"geometry": {
				"width": textWidth,
			},
		}

		response = requests.post(textsUrl, json=payload, headers=headers, timeout=30)

		if response.ok:
			print(f"Uploaded text '{textContent}' with fontSize {fontSize}: {response.status_code}")
			continue

		print(
			f"Failed to upload text '{textContent}': "
			f"{response.status_code} {response.text}"
		)


if __name__ == "__main__":
	uploadTexts()
