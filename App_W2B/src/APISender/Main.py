import json
import requests

API_TOKEN = 'eyJtaXJvLm9yaWdpbiI6ImV1MDEifQ_JIhCDDG4eg1ch_5_XsYIOoAp-_E'
BOARD_ID = 'uXjVG_yeA5I=' 
URL = f"https://api.miro.com/v2/boards/{BOARD_ID}/shapes"

def upload_shapes(json_path):
    with open(json_path, 'r') as f:
        shapes = json.load(f)

    headers = {
        "accept": "application/json",
        "content-type": "application/json",
        "authorization": f"Bearer {API_TOKEN}"
    }

    for shape in shapes:
        payload = {
            "data": {"shape": "rectangle"}, 
            "position": {
                "x": shape['x'], 
                "y": shape['y']
            },
            "style": {
                "fillColor": "#ffffff",
                "fillOpacity": "0.0",
                "fontFamily": "arial",
                "fontSize": "14",
                "textAlign": "center",
                "textAlignVertical": "middle",
                "borderStyle": "normal",
                "borderColor": "#000000",
                "borderOpacity": "1.0",
                "borderWidth": "2.0"

            },
            "geometry": {
                "width": shape.get('width', 100),
                "height": shape.get('height', 100)
            }
            }
        
        response = requests.post(URL, json=payload, headers=headers)
        print(f"Uploaded shape: {response.status_code}")

if __name__ == "__main__":
    upload_shapes("../json/detected_objects.json")