from pathlib import Path


GOOGLE_VISION_API_KEY = "AIzaSyBe-G8nctRHZdRkWcltqKr7cEHFen0m05cII"
MIRO_API_TOKEN = "eyJtaXJvLm9yaWdpbiI6ImV1MDEifQ_JIhCDDG4eg1ch_5_XsYIOoAp-_E"
MIRO_BOARD_ID = "uXjVG_yeA5I="

APP_ROOT = Path(__file__).resolve().parents[2]
JSON_DIR = APP_ROOT / "json"
SHAPES_JSON_PATH = JSON_DIR / "detectedObjects.json"
TEXT_JSON_PATH = JSON_DIR / "detectedText.json"