from ConnectorAPI import uploadConnectors
from ShapeAPI import uploadShapes
from TextAPI import uploadTexts


def main():
    uploadShapes() 
    uploadConnectors()
    uploadTexts()


if __name__ == "__main__":
    main()