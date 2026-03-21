import json
from pathlib import Path

import requests

from Config import MIRO_API_TOKEN, MIRO_BOARD_ID, SHAPES_JSON_PATH


SHAPES_URL = f"https://api.miro.com/v2/boards/{MIRO_BOARD_ID}/shapes"
DEFAULT_SHAPE_SIZE = 100
SHAPE_TYPE_MAP = {
	"rectangle": "rectangle",
	"circle": "circle",
	"triangle": "triangle",
}


def _build_headers(api_token): # Build the HTTP headers for API requests, including authorization using the provided API token
	return {
		"accept": "application/json",
		"content-type": "application/json",
		"authorization": f"Bearer {api_token}",
	}


def _shape_center(shape): # Calculate the center coordinates and dimensions of a shape based on its properties
	width = shape.get("width", DEFAULT_SHAPE_SIZE)
	height = shape.get("height", DEFAULT_SHAPE_SIZE)
	center_x = shape.get("x", 0) + width / 2
	center_y = shape.get("y", 0) + height / 2
	return center_x, center_y, width, height


def upload_shapes(json_path=SHAPES_JSON_PATH, api_token=MIRO_API_TOKEN): # Upload shapes to Miro board using the provided JSON file and API token
	json_path = Path(json_path)
	if not json_path.exists():
		print(f"Shapes JSON not found: {json_path}")
		return

	with json_path.open("r", encoding="utf-8") as file:
		shapes = json.load(file)

	headers = _build_headers(api_token)

	for shape in shapes:
		center_x, center_y, width, height = _shape_center(shape)
		miro_shape_type = SHAPE_TYPE_MAP.get(shape.get("type", "rectangle"), "rectangle")

		payload = {
			"data": {"shape": miro_shape_type},
			"position": {
				"x": center_x,
				"y": center_y,
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

		response = requests.post(SHAPES_URL, json=payload, headers=headers, timeout=30)

		if response.ok:
			print(f"Uploaded shape {miro_shape_type}: {response.status_code}")
			continue

		print(
			f"Failed to upload shape {miro_shape_type}: "
			f"{response.status_code} {response.text}"
		)


if __name__ == "__main__":
	upload_shapes()
