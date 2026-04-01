import json
from pathlib import Path

import requests

from Config import miroApiToken, miroBoardId, shapesJsonPath


shapesUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/shapes"
stickyNoteUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/sticky_notes"
defaultShapeSize = 100
shapeTypeMap = {
	"rectangle": "rectangle",
	"circle": "circle",
	"triangle": "triangle",
	"stickyNote": "stickyNote",
	"sticky_note": "stickyNote",
}

STICKY_NOTE_COLOR_MAP = {
	"0,0,255": "red",     
	"0,255,0": "green",     
	"255,0,0": "blue",         
	"0,0,0": "black",           
	"0,165,255": "orange",      
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


def hexToRgb(hexColor): 
	hexColor = hexColor.lstrip('#')
	if len(hexColor) == 6:
		return tuple(int(hexColor[i:i+2], 16) for i in (0, 2, 4))
	return (255, 255, 255) 


def mapColorHexToMiroStickyNoteColor(hexColor): # Map hex color to Miro sticky note color
	hexColor = hexColor.lstrip('#')
	if len(hexColor) == 6:
		r = int(hexColor[0:2], 16)
		g = int(hexColor[2:4], 16)
		b = int(hexColor[4:6], 16)
		bgrKey = f"{b},{g},{r}"
		
		return STICKY_NOTE_COLOR_MAP.get(bgrKey, "yellow")  
	
	return "yellow"  


def saveShapes(jsonPath, shapes):
	with Path(jsonPath).open("w", encoding="utf-8") as file:
		json.dump(shapes, file, ensure_ascii=False, indent=2)


def uploadShapes(jsonPath=shapesJsonPath, apiToken=miroApiToken, frameId=None, frameOffsetX=0.0, frameOffsetY=0.0): # Main function to read detected shapes from JSON and upload them to Miro board via API
	jsonPath = Path(jsonPath)
	if not jsonPath.exists():
		print(f"Shapes JSON not found: {jsonPath}")
		return

	with jsonPath.open("r", encoding="utf-8") as file:
		shapes = json.load(file)

	headers = buildHeaders(apiToken)

	for shape in shapes:
		shape.pop("miroId", None)
		centerX, centerY, width, height = getShapeCenter(shape)
		if frameId:
			centerX -= frameOffsetX
			centerY -= frameOffsetY
		miroShapeType = shapeTypeMap.get(shape.get("type", "rectangle"), "rectangle")

		if miroShapeType == "stickyNote":
			detectedHexColor = shape.get("colorHex", "#000000")
			miroStickyNoteColor = mapColorHexToMiroStickyNoteColor(detectedHexColor)
			
			payload = {
				"data": {
					"content": "",  
					"shape": "square"
				},
				"style": {
					"fillColor": miroStickyNoteColor,  
				},
				"position": {
					"x": centerX,
					"y": centerY,
				},
				"geometry": {
					"width": width if width > 0 else 100,
				},
				**({"parent": {"id": frameId}} if frameId else {}),
			}
		else:
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
				**({"parent": {"id": frameId}} if frameId else {}),
			}

		if miroShapeType == "stickyNote":
			response = requests.post(stickyNoteUrl, json=payload, headers=headers, timeout=30)
		else:
			response = requests.post(shapesUrl, json=payload, headers=headers, timeout=30)

		if response.ok:
			try:
				responsePayload = response.json()
			except ValueError:
				responsePayload = {}

			miroId = responsePayload.get("id")
			if miroId:
				shape["miroId"] = miroId
			print(
				f"Uploaded shape {miroShapeType}: {response.status_code}, "
				f"miroId={miroId}"
			)
			continue

		print(
			f"Failed to upload shape {miroShapeType}: "
			f"{response.status_code} {response.text}"
		)

	saveShapes(jsonPath, shapes)


if __name__ == "__main__":
	uploadShapes()
