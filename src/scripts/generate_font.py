import urllib.request
import re

URL = "https://raw.githubusercontent.com/torvalds/linux/master/lib/fonts/font_8x8.c"
OUTPUT_FILE = "src/particles/FontData.h"

print(f"Downloading {URL}...")
try:
    with urllib.request.urlopen(URL) as response:
        content = response.read().decode('utf-8')
except Exception as e:
    print(f"Error downloading: {e}")
    exit(1)

print("Parsing content...")
# regex to find the inner array data
# typically inside "fontdata_8x8 = { ... { DATA } ... };"
# We look for the second opening brace {
match = re.search(r"fontdata_8x8\s*=\s*\{\s*\{.*?\}\s*,\s*\{(.*?)\}\s*\}\s*;", content, re.DOTALL)

if not match:
    print("Error: Could not find fontdata_8x8 array in content.")
    # Fallback debug
    print("Content snippet:")
    print(content[:500])
    exit(1)

raw_data = match.group(1)

# Remove comments
raw_data = re.sub(r"/\*.*?\*/", "", raw_data, flags=re.DOTALL)
# Remove newlines and extra spaces
raw_data = raw_data.replace("\n", " ").replace("\t", "")

# Extract hex values
hex_values = re.findall(r"0x[0-9a-fA-F]{2}", raw_data)

print(f"Found {len(hex_values)} bytes.")

if len(hex_values) != 256 * 8:
    print(f"Warning: Expected 2048 bytes, found {len(hex_values)}")

# Format output
print(f"Writing to {OUTPUT_FILE}...")

with open(OUTPUT_FILE, "w") as f:
    f.write("#pragma once\n\n")
    f.write("#include <cstdint>\n\n")
    f.write("// generated from linux/lib/fonts/font_8x8.c\n")
    f.write("constexpr uint8_t PROGMEM_FONT8x8[] = {\n")
    
    for i, val in enumerate(hex_values):
        if i % 8 == 0:
            f.write(f"\n    // Char {i//8}\n    ")
        f.write(f"{val}, ")
        
    f.write("\n};\n")

print("Done!")
