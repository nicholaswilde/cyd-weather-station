#!/usr/bin/env python3
import os
import glob
from PIL import Image

def main():
    screenshots_dir = "screenshots"
    if not os.path.isdir(screenshots_dir):
        print(f"Directory '{screenshots_dir}' does not exist.")
        return

    bmp_pattern = os.path.join(screenshots_dir, "*.bmp")
    bmp_files = glob.glob(bmp_pattern)

    for bmp_path in bmp_files:
        png_path = os.path.splitext(bmp_path)[0] + ".png"
        print(f"Converting {bmp_path} to {png_path}...")
        try:
            with Image.open(bmp_path) as img:
                img.save(png_path, "PNG")
            os.remove(bmp_path)
            print(f"Successfully converted {os.path.basename(bmp_path)} to PNG.")
        except Exception as e:
            print(f"Failed to convert {os.path.basename(bmp_path)}: {e}")

if __name__ == "__main__":
    main()
