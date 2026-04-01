import json
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, textJsonPath
from Config import shapesJsonPath


textsUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/texts"
defaultTextWidth = 120
minFontSize = 10
FontSize2 = 18
FontSize3 = 24
FontSize4 = 36
FontSize5 = 48
FontSize6 = 60
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


def loadDetectedShapes(jsonPath=shapesJsonPath): # Load the detected shapes from the JSON file
	jsonPath = Path(jsonPath)
	if not jsonPath.exists():
		return []

	with jsonPath.open("r", encoding="utf-8") as file:
		return json.load(file)


def intersectionOverArea(ax, ay, aw, ah, bx, by, bw, bh): # Calculate the intersection area over the area of the first rectangle
	left = max(ax, bx)
	top = max(ay, by)
	right = min(ax + aw, bx + bw)
	bottom = min(ay + ah, by + bh)

	if right <= left or bottom <= top:
		return 0.0

	interArea = (right - left) * (bottom - top)
	aArea = max(1.0, aw * ah)
	return interArea / aArea


def shouldSkipTextBlock(textContent, minX, minY, textWidth, textHeight, shapes): # Determine if a text block should be skipped based on overlap with detected shapes
	visibleChars = len("".join(textContent.split()))
	if visibleChars == 0:
		return True

	for shape in shapes:
		shapeType = shape.get("type", "")
		sx = float(shape.get("x", 0))
		sy = float(shape.get("y", 0))
		sw = float(shape.get("width", 0))
		sh = float(shape.get("height", 0))

		if sw <= 0 or sh <= 0:
			continue

		overlapOnText = intersectionOverArea(minX, minY, textWidth, textHeight, sx, sy, sw, sh)
		overlapOnShape = intersectionOverArea(sx, sy, sw, sh, minX, minY, textWidth, textHeight)

		shapeArea = sw * sh
		textArea = max(1.0, textWidth * textHeight)
		areaRatio = shapeArea / textArea

		if shapeType == "circle" and visibleChars == 1:
			if overlapOnText > 0.60 and overlapOnShape > 0.35 and 0.70 <= areaRatio <= 1.80:
				return True

		if visibleChars <= 2 and overlapOnText > 0.75 and areaRatio < 2.20:
			return True

	return False


def reconstructWordText(word): # Reconstruct the text of a word from its symbols
	text = ""
	for symbol in word.get("symbols", []):
		text += symbol.get("text", "")
		breakType = symbol.get("property", {}).get("detectedBreak", {}).get("type", "")
		if breakType in ("SPACE", "EOL_SURE_SPACE"):
			text += " "
		elif breakType == "LINE_BREAK":
			text += "\n"
	return text


def reconstructBlockText(block):
	parts = []
	for paragraph in block.get("paragraphs", []):
		paragraphText = "".join(reconstructWordText(w) for w in paragraph.get("words", [])).strip()
		if paragraphText:
			parts.append(paragraphText)
	return "\n".join(parts)


def readTextBlocks(jsonPath): # Read the detected text blocks from the JSON
	with Path(jsonPath).open("r", encoding="utf-8") as file:
		payload = json.load(file)

	responses = payload.get("responses", [])
	if not responses:
		return []

	pages = responses[0].get("fullTextAnnotation", {}).get("pages", [])
	if not pages:
		return []

	return pages[0].get("blocks", [])


def calculateFontSize(textContent, textWidth, textHeight): # calculate font and pick one of the predefined sizes
	if textWidth <= 0 or textHeight <= 0:
		return minFontSize

	aspectRatio = textWidth / textHeight if textHeight > 0 else 1
	if aspectRatio > 5:
		return FontSize2
	elif aspectRatio > 3:
		return FontSize3
	elif aspectRatio > 1.5:
		return FontSize4
	elif aspectRatio > 1:
		return FontSize5
	else:
		return FontSize6
	


def uploadTexts(jsonPath=textJsonPath, apiToken=miroApiToken, frameId=None, frameOffsetX=0.0, frameOffsetY=0.0): # Main function to read detected text blocks from JSON and upload them to Miro board via API
	jsonPath = Path(jsonPath)
	if not jsonPath.exists():
		print(f"Text JSON not found: {jsonPath}")
		return

	blocks = readTextBlocks(jsonPath)
	if not blocks:
		print("No text blocks found for upload.")
		return

	shapes = loadDetectedShapes()

	headers = buildHeaders(apiToken)

	for block in blocks:
		textContent = reconstructBlockText(block)
		if not textContent:
			continue

		vertices = block.get("boundingBox", {}).get("vertices", [])
		minX, minY, maxX, maxY = extractBounds(vertices)
		textWidth = max(defaultTextWidth, maxX - minX)
		textHeight = max(1, maxY - minY)
		centerX = minX + textWidth / 2
		centerY = minY + textHeight / 2
		if frameId:
			centerX -= frameOffsetX
			centerY -= frameOffsetY
		fontSize = calculateFontSize(textContent.split("\n")[0], textWidth, textHeight)

		if shouldSkipTextBlock(textContent, minX, minY, textWidth, textHeight, shapes):
			continue

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
			**({"parent": {"id": frameId}} if frameId else {}),
		}

		response = requests.post(textsUrl, json=payload, headers=headers, timeout=30)

		if response.ok:
			preview = textContent.split("\n")[0][:40]
			continue

		preview = textContent.split("\n")[0][:40]
		print(
			f"Failed to upload text '{preview}': "
			f"{response.status_code} {response.text}"
		)


if __name__ == "__main__":
	uploadTexts()
