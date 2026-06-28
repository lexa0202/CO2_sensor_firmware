#!/usr/bin/env python3
"""
===============================================================================

Adafruit GFX Font Converter

Converts original Adafruit GFX fonts into STM32-friendly
.c/.h files used by the CO2 Sensor Firmware project.

Input:

    font_input/
        FreeSans9pt7b.h
        FreeSans12pt7b.h

Output:

    font_output/
        FreeSans9pt7b.c
        FreeSans9pt7b.h

===============================================================================
"""

from pathlib import Path
import re
print("SCRIPT STARTED")

###############################################################################
# Configuration
###############################################################################

INPUT_DIR = Path("font_input")
OUTPUT_DIR = Path("font_output")

###############################################################################
# Console
###############################################################################

def print_header():

    print()
    print("=" * 70)
    print(" Adafruit GFX Font Converter")
    print("=" * 70)
    print()

def print_summary(converted):

    print()
    print("=" * 70)
    print(f"Generated fonts : {converted}")
    print(f"Output folder   : {OUTPUT_DIR}")
    print("=" * 70)
    print()

###############################################################################
# Helpers
###############################################################################

def ensure_output_directory():

    OUTPUT_DIR.mkdir(exist_ok=True)

def read_text(path: Path) -> str:

    with open(path, "r", encoding="utf-8") as f:
        return f.read()

def write_text(path: Path, text: str):

    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)

def font_name(path: Path) -> str:

    return path.stem
###############################################################################
# Font parser
###############################################################################

def extract_array(text: str, array_name: str) -> str:
    """
    Extract C array body.

    Example:

        const uint8_t Name[] PROGMEM =
        {
            ...
        };

    Returns only the contents between '{' and '}'.
    """

    pattern = (
        rf"{array_name}\[\]\s*"
        r"(?:PROGMEM\s*)?"
        r"=\s*"
        r"\{(.*?)\};"
    )

    match = re.search(
        pattern,
        text,
        re.DOTALL
    )

    if match is None:
        raise RuntimeError(
            f"Cannot find array '{array_name}'"
        )

    return match.group(1).strip()


def extract_font_descriptor(
    text: str,
    font_name: str
) -> str:
    """
    Extract GFXfont structure.
    """

    pattern = (
        rf"{font_name}\s*"
        r"(?:PROGMEM\s*)?"
        r"=\s*"
        r"\{(.*?)\};"
    )

    match = re.search(
        pattern,
        text,
        re.DOTALL
    )

    if match is None:
        raise RuntimeError(
            f"Cannot find font '{font_name}'"
        )

    return match.group(1).strip()


def parse_font(path: Path) -> dict:
    """
    Parse original Adafruit font.

    Returns dictionary with:

        name
        bitmaps
        glyphs
        descriptor
    """

    text = read_text(path)

    name = font_name(path)

    bitmaps = extract_array(
        text,
        f"{name}Bitmaps"
    )

    glyphs = extract_array(
        text,
        f"{name}Glyphs"
    )

    descriptor = extract_font_descriptor(
        text,
        name
    )

    return {
        "name": name,
        "bitmaps": bitmaps,
        "glyphs": glyphs,
        "descriptor": descriptor
    }
###############################################################################
# Header generator
###############################################################################

def make_include_guard(font_name: str) -> str:
    """
    Generate include guard.

    Example:

        FreeSans9pt7b

    becomes

        FREESANS9PT7B_H
    """

    return f"{font_name.upper()}_H"


def generate_header(font: dict) -> str:
    """
    Generate .h file.
    """

    name = font["name"]

    guard = make_include_guard(name)

    return f'''/******************************************************************************
 * {name}.h
 *
 * Auto-generated from Adafruit GFX font.
 *
 * Source:
 *      {name}.h
 *
 * Do not edit manually.
 ******************************************************************************/

#ifndef {guard}
#define {guard}

#include "gfxfont.h"

#ifdef __cplusplus
extern "C"
{{
#endif

/******************************************************************************
 * Public font data
 ******************************************************************************/

extern const uint8_t {name}Bitmaps[];

extern const GFXglyph {name}Glyphs[];

extern const GFXfont {name};

#ifdef __cplusplus
}}
#endif

#endif /* {guard} */
'''
###############################################################################
# Source generator
###############################################################################

def format_block(title: str) -> str:
    """
    Generate formatted C comment block.
    """

    return (
        "/******************************************************************************\n"
        f" * {title}\n"
        " ******************************************************************************/\n\n"
    )


def format_bitmap(bitmap: str) -> str:
    """
    Beautify bitmap array.

    The original Adafruit fonts usually contain one long line.
    We split it into readable rows.
    """

    values = [
        value.strip()
        for value in bitmap.replace("\n", "").split(",")
        if value.strip()
    ]

    lines = []

    line = []

    for value in values:

        line.append(value)

        if len(line) == 16:
            lines.append("    " + ", ".join(line) + ",")
            line.clear()

    if line:
        lines.append("    " + ", ".join(line))

    return "\n".join(lines)


def format_glyphs(glyphs: str) -> str:
    """
    Beautify glyph table.
    """

    glyphs = glyphs.replace("},", "},\n")

    lines = []

    for line in glyphs.splitlines():

        line = line.strip()

        if line:
            lines.append("    " + line)

    return "\n".join(lines)


def format_descriptor(descriptor: str) -> str:
    """
    Beautify GFXfont descriptor.
    """

    descriptor = descriptor.replace(",", ",\n")

    lines = []

    for line in descriptor.splitlines():

        line = line.strip()

        if line:
            lines.append("    " + line)

    return "\n".join(lines)
def generate_source(font: dict) -> str:
    """
    Generate source (.c) file.
    """

    name = font["name"]

    bitmap = format_bitmap(
        font["bitmaps"]
    )

    glyphs = format_glyphs(
        font["glyphs"]
    )

    descriptor = format_descriptor(
        font["descriptor"]
    )

    text = ""

    text += "/******************************************************************************\n"
    text += f" * {name}.c\n"
    text += " *\n"
    text += " * Auto-generated from Adafruit GFX font.\n"
    text += " *\n"
    text += " * Do not edit manually.\n"
    text += " ******************************************************************************/\n\n"

    text += f'#include "{name}.h"\n\n'

    #
    # Bitmap
    #

    text += format_block("Bitmap data")

    text += (
        f"const uint8_t {name}Bitmaps[] =\n"
        "{\n"
    )

    text += bitmap

    text += "\n};\n\n"

    #
    # Glyphs
    #

    text += format_block("Glyph table")

    text += (
        f"const GFXglyph {name}Glyphs[] =\n"
        "{\n"
    )

    text += glyphs

    text += "\n};\n\n"

    #
    # Font descriptor
    #

    text += format_block("Font descriptor")

    text += (
        f"const GFXfont {name} =\n"
        "{\n"
    )

    text += descriptor

    text += "\n};\n"

    return text
###############################################################################
# Font conversion
###############################################################################

def convert_font(path: Path):
    """
    Convert a single Adafruit GFX font.
    """

    print(f"Converting {path.name}...")

    #
    # Parse source font
    #

    font = parse_font(path)

    #
    # Generate output files
    #

    header = generate_header(font)

    source = generate_source(font)

    #
    # Output filenames
    #

    header_path = OUTPUT_DIR / f"{font['name']}.h"

    source_path = OUTPUT_DIR / f"{font['name']}.c"

    #
    # Write files
    #

    write_text(
        header_path,
        header
    )

    write_text(
        source_path,
        source
    )

    print(f"  -> {header_path.name}")

    print(f"  -> {source_path.name}")

    print()

###############################################################################
# Main
###############################################################################

def main():

    print_header()

    ensure_output_directory()

    if not INPUT_DIR.exists():
        print(f"Input directory not found: {INPUT_DIR}")

        return

    fonts = sorted(
        INPUT_DIR.glob("*.h")
    )

    if not fonts:
        print("No Adafruit fonts found.")

        return

    converted = 0

    for font in fonts:

        try:

            convert_font(font)

            converted += 1

        except Exception as e:

            print(f"ERROR: {font.name}")

            print(f"       {e}")

            print()

    print_summary(converted)

if __name__ == "__main__":
    main()