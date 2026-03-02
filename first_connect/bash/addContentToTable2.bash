#!/bin/bash

# Nahraďte svými skutečnými údaji
BOARD_ID="uXjVG_yeA5I="
TOKEN="eyJtaXJvLm9yaWdpbiI6ImV1MDEifQ_JIhCDDG4eg1ch_5_XsYIOoAp-_E"

curl --request POST \
     --url "https://api.miro.com/v2/boards/${BOARD_ID}/shapes" \
     --header 'accept: application/json' \
     --header "authorization: Bearer ${TOKEN}" \
     --header 'content-type: application/json' \
     --data '
{
  "data": {
    "shape": "rectangle"
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
  "position": {
    "x": 500.0,
    "y": 500.0
  },
  "geometry": {
    "width": 200.0,
    "rotation": 50.0,
    "height": 200.0
  }
}'