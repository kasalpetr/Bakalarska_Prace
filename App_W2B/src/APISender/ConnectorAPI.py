import json
from pathlib import Path
from urllib import error, request

from Config import edgesJsonPath, miroApiToken, miroBoardId, shapesJsonPath


connectorsUrl = f"https://api.miro.com/v2/boards/{miroBoardId}/connectors"


def buildHeaders(apiToken):
	return {
		"accept": "application/json",
		"content-type": "application/json",
		"authorization": f"Bearer {apiToken}",
	}


def loadJsonArray(jsonPath):
	jsonPath = Path(jsonPath)
	if not jsonPath.exists():
		return []

	with jsonPath.open("r", encoding="utf-8") as file:
		return json.load(file)


def saveEdges(jsonPath, edges):
	with Path(jsonPath).open("w", encoding="utf-8") as file:
		json.dump(edges, file, ensure_ascii=False, indent=2)


def postJson(url, payload, headers, timeout=30):
	encodedPayload = json.dumps(payload).encode("utf-8")
	requestHeaders = dict(headers)
	req = request.Request(url, data=encodedPayload, headers=requestHeaders, method="POST")

	try:
		with request.urlopen(req, timeout=timeout) as response:
			responseBody = response.read().decode("utf-8")
			return response.status, responseBody
	except error.HTTPError as exc:
		responseBody = exc.read().decode("utf-8", errors="replace")
		return exc.code, responseBody


def buildConnectorPayload(startMiroId, endMiroId):
	return {
		"startItem": {
			"id": startMiroId,
			"snapTo": "auto",
		},
		"endItem": {
			"id": endMiroId,
			"snapTo": "auto",
		},
		"shape": "straight",
		"style": {
			"startStrokeCap": "none",
			"endStrokeCap": "none",
		},
	}


def uploadConnectors(edgesPath=edgesJsonPath, shapesPath=shapesJsonPath, apiToken=miroApiToken):
	edgesPath = Path(edgesPath)
	if not edgesPath.exists():
		print(f"Edges JSON not found: {edgesPath}")
		return

	edges = loadJsonArray(edgesPath)
	if not edges:
		print("No edges")
		return

	shapes = loadJsonArray(shapesPath)
	if not shapes:
		print("No shapes")
		return

	miroIdByShapeId = {
		shape.get("id"): shape.get("miroId")
		for shape in shapes
		if shape.get("id") is not None and shape.get("miroId")
	}

	headers = buildHeaders(apiToken)

	for edge in edges:
		edge.pop("startMiroId", None)
		edge.pop("endMiroId", None)
		edge.pop("miroConnectorId", None)

		startMiroId = miroIdByShapeId.get(edge.get("sourceShapeId"))
		endMiroId = miroIdByShapeId.get(edge.get("targetShapeId"))
		if not startMiroId or not endMiroId or startMiroId == endMiroId:
			print(
				f"sourceShapeId={edge.get('sourceShapeId')} and targetShapeId={edge.get('targetShapeId')}"
			)
			continue

		payload = buildConnectorPayload(startMiroId, endMiroId)
		statusCode, responseBody = postJson(connectorsUrl, payload, headers, timeout=30)

		if 200 <= statusCode < 300:
			try:
				responsePayload = json.loads(responseBody) if responseBody else {}
			except ValueError:
				responsePayload = {}

			edge["startMiroId"] = startMiroId
			edge["endMiroId"] = endMiroId
			connectorId = responsePayload.get("id")
			if connectorId:
				edge["miroConnectorId"] = connectorId

			print(
				f"Uploaded connector: {statusCode}, "
				f"start={startMiroId}, end={endMiroId}, "
				f"connectorId={connectorId}"
			)
			continue

		print(
			f"Failed to upload connector: {statusCode} {responseBody}"
		)

	saveEdges(edgesPath, edges)


if __name__ == "__main__":
	uploadConnectors()