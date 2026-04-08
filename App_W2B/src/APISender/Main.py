from ConnectorAPI import uploadConnectors
from FrameAPI import uploadFrame
from ImageAPI import uploadImage, uploadSourceImage
from ShapeAPI import uploadShapes
from TextAPI import uploadTexts


def main():
    frameId, frameTopLeftX, frameTopLeftY, frameCenterX, frameCenterY, frameWidth, frameHeight = uploadFrame()
    uploadImage(frameId=frameId, frameOffsetX=frameTopLeftX, frameOffsetY=frameTopLeftY)
    uploadShapes(frameId=frameId, frameOffsetX=frameTopLeftX, frameOffsetY=frameTopLeftY)
    uploadConnectors()
    uploadTexts(frameId=frameId, frameOffsetX=frameTopLeftX, frameOffsetY=frameTopLeftY)
    uploadSourceImage(
        frameCenterX=frameCenterX,
        frameCenterY=frameCenterY,
        frameHeight=frameHeight,
    )


if __name__ == "__main__":
    main()