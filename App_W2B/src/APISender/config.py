from pathlib import Path


googleVisionApiKey = "AIzaSyBe-G8nctRHZdRkWcltqKr7cEHFen0m05c"
miroApiToken = "eyJtaXJvLm9yaWdpbiI6ImV1MDEifQ_JIhCDDG4eg1ch_5_XsYIOoAp-_E"
miroBoardId = "uXjVG_yeA5I="

appRoot = Path(__file__).resolve().parents[2]
jsonDir = appRoot / "json"
shapesJsonPath = jsonDir / "detectedShapes.json"
textJsonPath = jsonDir / "detectedText.json"