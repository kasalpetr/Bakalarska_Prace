from ConnectorAPI import uploadConnectors
from FrameAPI import uploadFrame
from ShapeAPI import uploadShapes
from TextAPI import uploadTexts


def main():
    frameId, frameTopLeftX, frameTopLeftY = uploadFrame()
    uploadShapes(frameId=frameId, frameOffsetX=frameTopLeftX, frameOffsetY=frameTopLeftY)
    uploadConnectors()
    uploadTexts(frameId=frameId, frameOffsetX=frameTopLeftX, frameOffsetY=frameTopLeftY)


if __name__ == "__main__":
    main()