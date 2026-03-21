import json
from pathlib import Path

import requests

from Config import MIRO_API_TOKEN, MIRO_BOARD_ID, TEXT_JSON_PATH


TEXTS_URL = f"https://api.miro.com/v2/boards/{MIRO_BOARD_ID}/texts"
DEFAULT_TEXT_WIDTH = 120


def _build_headers(api_token): # Build the HTTP headers for API requests, including authorization using the provided API token
	return {
		"accept": "application/json",
		"content-type": "application/json",
		"authorization": f"Bearer {api_token}",
	}


def _extract_bounds(vertices): # Extract the minimum and maximum x and y coordinates from a list of vertices to determine the bounding box of a text annotation
	x_values = [vertex.get("x", 0) for vertex in vertices]
	y_values = [vertex.get("y", 0) for vertex in vertices]

	min_x = min(x_values, default=0)
	max_x = max(x_values, default=min_x)
	min_y = min(y_values, default=0)
	max_y = max(y_values, default=min_y)

	return min_x, min_y, max_x, max_y


def _read_text_annotations(json_path): # Read text annotations from the specified JSON file and return a list of annotations
	with Path(json_path).open("r", encoding="utf-8") as file:
		payload = json.load(file)

	responses = payload.get("responses", [])
	if not responses:
		return []

	annotations = responses[0].get("textAnnotations", [])
	return annotations[1:] if len(annotations) > 1 else []


def upload_texts(json_path=TEXT_JSON_PATH, api_token=MIRO_API_TOKEN): # Upload text annotations to Miro board using the provided JSON file and API token
	json_path = Path(json_path)
	if not json_path.exists():
		print(f"Text JSON not found: {json_path}")
		return

	annotations = _read_text_annotations(json_path)
	if not annotations:
		print("No text annotations found for upload.")
		return

	headers = _build_headers(api_token)

	for annotation in annotations:
		text_content = annotation.get("description", "").strip()
		if not text_content:
			continue

		vertices = annotation.get("boundingPoly", {}).get("vertices", [])
		min_x, min_y, max_x, max_y = _extract_bounds(vertices)
		width = max(DEFAULT_TEXT_WIDTH, max_x - min_x)
		center_x = min_x + width / 2
		center_y = min_y + (max_y - min_y) / 2

		payload = {
			"data": {
				"content": text_content,
			},
			"position": {
				"x": center_x,
				"y": center_y,
			},
			"geometry": {
				"width": width,
			},
		}

		response = requests.post(TEXTS_URL, json=payload, headers=headers, timeout=30)

		if response.ok:
			print(f"Uploaded text '{text_content}': {response.status_code}")
			continue

		print(
			f"Failed to upload text '{text_content}': "
			f"{response.status_code} {response.text}"
		)


if __name__ == "__main__":
	upload_texts()
