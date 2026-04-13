import subprocess
import json
import sys
import tkinter as tk
from tkinter import ttk, simpledialog, messagebox
from pathlib import Path
import requests

appRoot = Path(__file__).resolve().parents[2]
imageConfigPath = appRoot / "json" / "imageConfig.json"
connectionConfigPath = appRoot / "json" / "connectionConfig.json"

selectedImagePath = None
boardNames = []
boardMap = {}
cancelledByUser = True


def loadJson(path):
    if not path.exists():
        return {}
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except (json.JSONDecodeError, OSError):
        return {}


def saveJson(path, data):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4)


def parseBoards(config):
    boards = config.get("boards", [])
    parsedNames = []
    parsedMap = {}

    if isinstance(boards, list):
        for board in boards:
            if not isinstance(board, dict):
                continue
            name = str(board.get("name", "")).strip()
            boardId = str(board.get("id", "")).strip()
            if not name or not boardId:
                continue
            if name not in parsedMap:
                parsedMap[name] = boardId
                parsedNames.append(name)

    legacyName = str(config.get("miroBoardName", "")).strip()
    legacyId = str(config.get("miroBoardId", "")).strip()
    if legacyName and legacyId and legacyName not in parsedMap:
        parsedMap[legacyName] = legacyId
        parsedNames.append(legacyName)

    return parsedNames, parsedMap

def rebuildBoardsFromUi(selectedName=None):
    global boardNames, boardMap
    boardMap = {name: boardMap[name] for name in boardNames if name in boardMap}
    boardNames = list(boardMap.keys())

    boardCombo["values"] = boardNames
    if selectedName and selectedName in boardMap:
        boardCombo.set(selectedName)
    elif boardNames:
        boardCombo.set(boardNames[0])
    else:
        boardCombo.set("")

# create board
def createBoard():
    global boardNames, boardMap

    token = miroTokenEntry.get().strip() or connectionConfig.get("miroApiToken", "")
    if not token:
        messagebox.showerror("Missing token", "Please enter the Miro API token first.")
        return

    boardName = simpledialog.askstring("New board", "Name of the new board:", parent=root)
    if boardName is None:
        return

    boardName = boardName.strip()
    if not boardName:
        messagebox.showerror("Invalid name", "Board name cannot be empty.")
        return

    response = requests.post(
        "https://api.miro.com/v2/boards",
        headers={
            "accept": "application/json",
            "content-type": "application/json",
            "authorization": f"Bearer {token}",
        },
        json={"name": boardName},
        timeout=30,
    )

    if not response.ok:
        messagebox.showerror("Board creation failed", f"{response.status_code}: {response.text}")
        return

    payload = response.json()
    createdBoardId = str(payload.get("id", "")).strip()
    createdBoardName = str(payload.get("name") or boardName).strip()
    if not createdBoardId or not createdBoardName:
        messagebox.showerror("Error", "Miro API did not return a valid board id/name.")
        return

    boardMap[createdBoardName] = createdBoardId
    boardNames = [name for name in boardNames if name != createdBoardName]
    boardNames.append(createdBoardName)
    rebuildBoardsFromUi(selectedName=createdBoardName)
    messagebox.showinfo("Done", f"Board '{createdBoardName}' was created.")


def pickImageFile():
    global selectedImagePath
    try:
        result = subprocess.run(
            [
                "zenity", "--file-selection",
                "--title=Select image",
                "--file-filter=Image files (jpg, jpeg, png, bmp, tiff) | *.jpg *.jpeg *.png",
            ],
            capture_output=True,
            text=True,
        )
        path = result.stdout.strip()
        if result.returncode != 0 or not path:
            return
    except FileNotFoundError:
        from tkinter import filedialog
        path = filedialog.askopenfilename(
            title="Select image",
            filetypes=[("Image files", "*.jpg *.jpeg *.png"), ("All files", "*.*")]
        )
        if not path:
            return

    selectedImagePath = path
    pathLabel.config(text=path, fg="#1a1a1a")
    confirmBtn.config(state=tk.NORMAL)


def confirm():
    global connectionConfig, cancelledByUser

    if not selectedImagePath:
        return

    imageConfig = loadJson(imageConfigPath)
    imageConfig["path"] = selectedImagePath
    saveJson(imageConfigPath, imageConfig)

    connectionConfig = loadJson(connectionConfigPath)
    enteredGoogleKey = googleVisionEntry.get().strip()
    enteredMiroToken = miroTokenEntry.get().strip()
    selectedBoardName = boardCombo.get().strip()
    selectedBoardId = boardMap.get(selectedBoardName, "")

    connectionConfig["googleVisionApiKey"] = enteredGoogleKey or connectionConfig.get("googleVisionApiKey", "")
    connectionConfig["miroApiToken"] = enteredMiroToken or connectionConfig.get("miroApiToken", "")
    connectionConfig["selectedBoardName"] = selectedBoardName or connectionConfig.get("selectedBoardName", "")
    connectionConfig["selectedBoardId"] = selectedBoardId or connectionConfig.get("selectedBoardId", "")

    connectionConfig["miroBoardName"] = connectionConfig.get("selectedBoardName", "")
    connectionConfig["miroBoardId"] = connectionConfig.get("selectedBoardId", "")

    connectionConfig["boards"] = [
        {"name": name, "id": boardMap[name]}
        for name in boardNames
        if name in boardMap
    ]

    saveJson(connectionConfigPath, connectionConfig)

    cancelledByUser = False
    root.destroy()


def cancel():
    root.destroy()


root = tk.Tk()
root.title("Board2Digital")
root.geometry("720x500")
root.resizable(False, False)
root.configure(bg="#f0f0f0")
root.protocol("WM_DELETE_WINDOW", cancel)

# --- Header ---
headerFrame = tk.Frame(root, bg="#2c2c2c", height=50)
headerFrame.pack(fill=tk.X)
tk.Label(headerFrame, text="DigitalBoard", font=("Segoe UI", 14, "bold"),
         bg="#2c2c2c", fg="white", pady=12).pack(side=tk.LEFT, padx=16)

# --- Image selection section ---
selectFrame = tk.LabelFrame(root, text="Image selection", font=("Segoe UI", 10),
                             bg="#f0f0f0", padx=12, pady=12)
selectFrame.pack(fill=tk.X, padx=20, pady=(16, 8))

selectBtn = tk.Button(selectFrame, text="Select image", font=("Segoe UI", 10),
                      command=pickImageFile, bg="#0078d4", fg="white",
                      relief=tk.FLAT, padx=12, pady=6, cursor="hand2")
selectBtn.pack(side=tk.LEFT)

pathLabel = tk.Label(selectFrame, text="No image selected", font=("Segoe UI", 9),
                     bg="#f0f0f0", fg="#888888", wraplength=380, anchor="w")
pathLabel.pack(side=tk.LEFT, padx=(12, 0), fill=tk.X, expand=True)


connectionConfig = loadJson(connectionConfigPath)

apiFrame = tk.LabelFrame(root, text="API keys", font=("Segoe UI", 10),
                         bg="#f0f0f0", padx=12, pady=12)
apiFrame.pack(fill=tk.X, padx=20, pady=8)

tk.Label(apiFrame, text="Google Vision API key", font=("Segoe UI", 9),
         bg="#f0f0f0").grid(row=0, column=0, sticky="w")
googleVisionEntry = tk.Entry(apiFrame, font=("Segoe UI", 9), width=72)
googleVisionEntry.grid(row=1, column=0, pady=(2, 10), sticky="ew")
googleVisionEntry.insert(0, connectionConfig.get("googleVisionApiKey", ""))

tk.Label(apiFrame, text="Miro API token", font=("Segoe UI", 9),
         bg="#f0f0f0").grid(row=2, column=0, sticky="w")
miroTokenEntry = tk.Entry(apiFrame, font=("Segoe UI", 9), width=72)
miroTokenEntry.grid(row=3, column=0, pady=(2, 10), sticky="ew")
miroTokenEntry.insert(0, connectionConfig.get("miroApiToken", ""))

tk.Label(apiFrame, text="Miro board", font=("Segoe UI", 9),
         bg="#f0f0f0").grid(row=4, column=0, sticky="w")

boardRowFrame = tk.Frame(apiFrame, bg="#f0f0f0")
boardRowFrame.grid(row=5, column=0, pady=(2, 0), sticky="ew")

boardCombo = ttk.Combobox(boardRowFrame, state="readonly", width=52)
boardCombo.pack(side=tk.LEFT, fill=tk.X, expand=True)

createBoardBtn = tk.Button(
    boardRowFrame,
    text="+ New board",
    font=("Segoe UI", 9),
    command=createBoard,
    bg="#005a9e",
    fg="white",
    relief=tk.FLAT,
    padx=10,
    pady=4,
    cursor="hand2",
)
createBoardBtn.pack(side=tk.LEFT, padx=(8, 0))

boardNames, boardMap = parseBoards(connectionConfig)
preferredBoardName = str(connectionConfig.get("selectedBoardName") or connectionConfig.get("miroBoardName") or "").strip()
rebuildBoardsFromUi(selectedName=preferredBoardName)

apiFrame.grid_columnconfigure(0, weight=1)

# --- Bottom bar ---
bottomFrame = tk.Frame(root, bg="#f0f0f0")
bottomFrame.pack(fill=tk.X, padx=20, pady=(8, 16), side=tk.BOTTOM)

confirmBtn = tk.Button(bottomFrame, text="Start conversion", font=("Segoe UI", 10, "bold"),
                       command=confirm, bg="#107c10", fg="white",
                       relief=tk.FLAT, padx=16, pady=8, cursor="hand2",
                       state=tk.DISABLED)
confirmBtn.pack(side=tk.RIGHT)

cancelBtn = tk.Button(bottomFrame, text="Cancel", font=("Segoe UI", 10),
                      command=cancel, bg="#777777", fg="white",
                      relief=tk.FLAT, padx=16, pady=8, cursor="hand2")
cancelBtn.pack(side=tk.RIGHT, padx=(0, 8))

existingImageConfig = loadJson(imageConfigPath)
existingImagePath = existingImageConfig.get("path", "")
if existingImagePath:
    selectedImagePath = existingImagePath
    pathLabel.config(text=existingImagePath, fg="#1a1a1a")
    confirmBtn.config(state=tk.NORMAL)


def runUI():
    root.mainloop()
    if cancelledByUser:
        print("Cancelled by user.", file=sys.stderr)
        sys.exit(1)
    if not selectedImagePath:
        print("No image selected.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    runUI()
