import subprocess
import json
import sys
import tkinter as tk
from pathlib import Path

appRoot = Path(__file__).resolve().parents[2]
imageConfigPath = appRoot / "json" / "imageConfig.json"

selectedImagePath = None


def pickImageFile():
    global selectedImagePath
    try:
        result = subprocess.run(
            [
                "zenity", "--file-selection",
                "--title=Vybrat fotku",
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
            title="Vybrat fotku",
            filetypes=[("Image files", "*.jpg *.jpeg *.png"), ("All files", "*.*")]
        )
        if not path:
            return

    selectedImagePath = path
    pathLabel.config(text=path, fg="#1a1a1a")
    confirmBtn.config(state=tk.NORMAL)


def confirm():
    if not selectedImagePath:
        return

    with open(imageConfigPath, "r") as f:
        config = json.load(f)

    config["path"] = selectedImagePath

    with open(imageConfigPath, "w") as f:
        json.dump(config, f, indent=4)

    root.destroy()


root = tk.Tk()
root.title("DigitalBoard")
root.geometry("600x350")
root.resizable(False, False)
root.configure(bg="#f0f0f0")

# --- Header ---
headerFrame = tk.Frame(root, bg="#2c2c2c", height=50)
headerFrame.pack(fill=tk.X)
tk.Label(headerFrame, text="DigitalBoard", font=("Segoe UI", 14, "bold"),
         bg="#2c2c2c", fg="white", pady=12).pack(side=tk.LEFT, padx=16)

# --- Image selection section ---
selectFrame = tk.LabelFrame(root, text="Výběr fotky", font=("Segoe UI", 10),
                             bg="#f0f0f0", padx=12, pady=12)
selectFrame.pack(fill=tk.X, padx=20, pady=(16, 8))

selectBtn = tk.Button(selectFrame, text="Vybrat fotku", font=("Segoe UI", 10),
                      command=pickImageFile, bg="#0078d4", fg="white",
                      relief=tk.FLAT, padx=12, pady=6, cursor="hand2")
selectBtn.pack(side=tk.LEFT)

pathLabel = tk.Label(selectFrame, text="Žádná fotka nevybrána", font=("Segoe UI", 9),
                     bg="#f0f0f0", fg="#888888", wraplength=380, anchor="w")
pathLabel.pack(side=tk.LEFT, padx=(12, 0), fill=tk.X, expand=True)

# --- Placeholder for future sections ---
futureFrame = tk.LabelFrame(root, text="Nastavení", font=("Segoe UI", 10),
                             bg="#f0f0f0", padx=12, pady=12)
futureFrame.pack(fill=tk.X, padx=20, pady=8)
tk.Label(futureFrame, text="(Next)", font=("Segoe UI", 9),
         bg="#f0f0f0", fg="#aaaaaa").pack(anchor="w")

# --- Bottom bar ---
bottomFrame = tk.Frame(root, bg="#f0f0f0")
bottomFrame.pack(fill=tk.X, padx=20, pady=(8, 16), side=tk.BOTTOM)

confirmBtn = tk.Button(bottomFrame, text="Spustit převod", font=("Segoe UI", 10, "bold"),
                       command=confirm, bg="#107c10", fg="white",
                       relief=tk.FLAT, padx=16, pady=8, cursor="hand2",
                       state=tk.DISABLED)
confirmBtn.pack(side=tk.RIGHT)


def runUI():
    root.mainloop()
    if not selectedImagePath:
        print("No image selected.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    runUI()
