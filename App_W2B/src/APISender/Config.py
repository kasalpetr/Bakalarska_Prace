import json
from pathlib import Path

appRoot = Path(__file__).resolve().parents[2]
jsonDir = appRoot / "json"
edgesJsonPath = jsonDir / "detectedEdges.json"
shapesJsonPath = jsonDir / "detectedShapes.json"
textJsonPath = jsonDir / "detectedText.json"
imageConfigJsonPath = jsonDir / "imageConfig.json"
residualImagePath = appRoot / "Img" / "residual.png"
connectionConfigJsonPath = jsonDir / "connectionConfig.json"


def _loadConnectionConfig():
	if not connectionConfigJsonPath.exists():
		return {}

	try:
		return json.loads(connectionConfigJsonPath.read_text(encoding="utf-8"))
	except (json.JSONDecodeError, OSError):
		return {}


_connectionConfig = _loadConnectionConfig()
googleVisionApiKey = _connectionConfig.get("googleVisionApiKey", "")
miroApiToken = _connectionConfig.get("miroApiToken", "")


def _resolveBoardId(config):
	selectedBoardId = str(config.get("selectedBoardId", "")).strip()
	if selectedBoardId:
		return selectedBoardId

	selectedBoardName = str(config.get("selectedBoardName") or config.get("miroBoardName") or "").strip()
	boards = config.get("boards", [])
	if selectedBoardName and isinstance(boards, list):
		for board in boards:
			if not isinstance(board, dict):
				continue
			boardName = str(board.get("name", "")).strip()
			boardId = str(board.get("id", "")).strip()
			if boardName == selectedBoardName and boardId:
				return boardId

	return str(config.get("miroBoardId", "")).strip()


miroBoardId = _resolveBoardId(_connectionConfig)