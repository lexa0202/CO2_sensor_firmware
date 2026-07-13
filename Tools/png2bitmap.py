#!/usr/bin/env python3
"""
PNG -> Bitmap converter.

Converts monochrome PNG images into Bitmap_t resources
used by the firmware UI.
"""

import argparse
from pathlib import Path

from PIL import Image


###############################################################################
# Configuration
###############################################################################

#
# Alpha / grayscale threshold.
#
THRESHOLD = 128

#
# Automatically crop transparent borders.
#
AUTO_CROP = True

#
# Center image after scaling.
#
CENTER_IMAGE = True

#
# Mask source:
#
# "alpha"      -> use alpha channel
# "grayscale"  -> use image brightness
#
MASK_SOURCE = "alpha"

#
# Background removal tolerance.
#
BG_TOLERANCE = 30

#
# Directories.
#
INPUT_DIR = Path("bitmap_input")

OUTPUT_DIR = Path("bitmap_output")


###############################################################################
# Utility
###############################################################################

def camel_case(text: str) -> str:
    """
    Convert snake_case into CamelCase.
    """

    return "".join(
        part.capitalize()
        for part in text.replace("-", "_").split("_")
    )


def ensure_output_directory():
    """
    Create output directory.
    """

    OUTPUT_DIR.mkdir(
        exist_ok=True
    )


def resource_name(
    name: str,
    width: int,
    height: int
) -> str:
    """
    Generate C resource name.
    """

    base = camel_case(name)

    if width == height:

        return (
            f"Bitmap{base}{width}"
        )

    return (
        f"Bitmap{base}"
        f"{width}x{height}"
    )


def bitmap_data_name(name: str) -> str:
    """
    Bitmap byte array name.
    """

    return (
        f"bitmap_{name}"
    )


def header_filename(name: str) -> str:
    """
    Header filename.
    """

    return (
        f"{name}.h"
    )


def source_filename(name: str) -> str:
    """
    Source filename.
    """

    return (
        f"{name}.c"
    )
###############################################################################
# Background removal
###############################################################################

NAMED_COLORS = {
    "white": (255, 255, 255),
    "black": (0, 0, 0),
}


def parse_color(value: str) -> tuple[int, int, int]:
    """
    Parse a background color from CLI input.
    """

    normalized = value.strip().lower()

    if normalized in NAMED_COLORS:
        return NAMED_COLORS[normalized]

    if normalized.startswith("#"):
        hex_value = normalized[1:]

        if len(hex_value) != 6:
            raise ValueError(
                f"Invalid color format: {value}"
            )

        return (
            int(hex_value[0:2], 16),
            int(hex_value[2:4], 16),
            int(hex_value[4:6], 16),
        )

    parts = normalized.split(",")

    if len(parts) == 3:
        return tuple(
            int(part.strip())
            for part in parts
        )

    raise ValueError(
        f"Invalid color format: {value}"
    )


def color_matches(
    red: int,
    green: int,
    blue: int,
    target: tuple[int, int, int],
    tolerance: int
) -> bool:
    """
    Check whether a pixel color matches the target within tolerance.
    """

    target_red, target_green, target_blue = target

    return (
        abs(red - target_red) <= tolerance and
        abs(green - target_green) <= tolerance and
        abs(blue - target_blue) <= tolerance
    )


def detect_background_color(
    image: Image.Image
) -> tuple[int, int, int]:
    """
    Detect background color from image corners.
    """

    width, height = image.size
    pixels = image.load()

    corners = [
        pixels[0, 0][:3],
        pixels[width - 1, 0][:3],
        pixels[0, height - 1][:3],
        pixels[width - 1, height - 1][:3],
    ]

    return max(
        set(corners),
        key=corners.count
    )


def remove_solid_background(
    image: Image.Image,
    bg_color: tuple[int, int, int] | None,
    tolerance: int
) -> Image.Image:
    """
    Remove a solid background using flood fill from image borders.
    """

    if bg_color is None:
        bg_color = detect_background_color(image)

    width, height = image.size
    pixels = image.load()
    visited = [
        [False] * width
        for _ in range(height)
    ]

    queue = []

    for x in range(width):
        queue.append((x, 0))
        queue.append((x, height - 1))

    for y in range(1, height - 1):
        queue.append((0, y))
        queue.append((width - 1, y))

    while queue:
        x, y = queue.pop()

        if visited[y][x]:
            continue

        red, green, blue, alpha = pixels[x, y]

        if alpha < THRESHOLD:
            continue

        if not color_matches(
            red,
            green,
            blue,
            bg_color,
            tolerance
        ):
            continue

        visited[y][x] = True
        pixels[x, y] = (
            red,
            green,
            blue,
            0
        )

        if x > 0:
            queue.append((x - 1, y))

        if x < width - 1:
            queue.append((x + 1, y))

        if y > 0:
            queue.append((x, y - 1))

        if y < height - 1:
            queue.append((x, y + 1))

    return image


###############################################################################
# Image loading
###############################################################################

def crop_transparent_borders(image: Image.Image) -> Image.Image:
    """
    Remove transparent borders from an RGBA image.
    """

    if not AUTO_CROP:
        return image

    bounds = image.getbbox()

    if bounds is None:
        return image

    return image.crop(bounds)


def scale_to_fit(
    image: Image.Image,
    target_width: int,
    target_height: int
) -> Image.Image:
    """
    Scale image to fit inside target size while preserving aspect ratio.
    """

    if image.width == 0 or image.height == 0:
        return image

    scale = min(
        target_width / image.width,
        target_height / image.height
    )

    new_width = max(
        1,
        round(image.width * scale)
    )

    new_height = max(
        1,
        round(image.height * scale)
    )

    if new_width == image.width and new_height == image.height:
        return image

    return image.resize(
        (
            new_width,
            new_height
        ),
        Image.Resampling.LANCZOS
    )


def place_on_canvas(
    image: Image.Image,
    target_width: int,
    target_height: int
) -> Image.Image:
    """
    Place image on a canvas of the requested size.
    """

    canvas = Image.new(
        "RGBA",
        (
            target_width,
            target_height
        ),
        (
            0,
            0,
            0,
            0
        )
    )

    if CENTER_IMAGE:
        offset_x = (
            target_width - image.width
        ) // 2

        offset_y = (
            target_height - image.height
        ) // 2

    else:
        offset_x = 0
        offset_y = 0

    canvas.paste(
        image,
        (
            offset_x,
            offset_y
        ),
        image
    )

    return canvas


def load_image(
    path: Path,
    target_width: int,
    target_height: int,
    remove_background: bool = True,
    bg_color: tuple[int, int, int] | None = None,
    bg_tolerance: int = BG_TOLERANCE
):
    """
    Load PNG image and convert it to a monochrome bitmap.

    Solid backgrounds can be removed first, then transparent borders are
    cropped, visible content is scaled to fit inside the requested size,
    and the result is centered on a canvas with the exact target dimensions.
    """

    image = Image.open(path).convert("RGBA")

    if remove_background:
        image = remove_solid_background(
            image,
            bg_color,
            bg_tolerance
        )

    image = crop_transparent_borders(image)

    if image.getbbox() is None:
        raise ValueError(
            "Image is fully transparent"
        )

    image = scale_to_fit(
        image,
        target_width,
        target_height
    )

    canvas = place_on_canvas(
        image,
        target_width,
        target_height
    )

    width = target_width
    height = target_height

    bitmap = []

    for y in range(height):

        row = []

        for x in range(width):

            red, green, blue, alpha = canvas.getpixel((x, y))

            if MASK_SOURCE == "alpha":

                pixel = (
                    alpha >= THRESHOLD
                )

            else:

                gray = (
                    299 * red +
                    587 * green +
                    114 * blue
                ) // 1000

                pixel = (
                    gray < THRESHOLD
                )

            row.append(
                1 if pixel else 0
            )

        bitmap.append(row)

    return (
        bitmap,
        width,
        height
    )
###############################################################################
# Bitmap packing
###############################################################################

def pack_bitmap(bitmap):
    """
    Pack bitmap into MSB-first byte array.
    """

    height = len(bitmap)

    width = len(bitmap[0])

    bytes_per_row = (
        width + 7
    ) // 8

    output = []

    for y in range(height):

        for byte_index in range(bytes_per_row):

            value = 0

            for bit in range(8):

                x = (
                    byte_index * 8
                ) + bit

                if x >= width:
                    continue

                if bitmap[y][x]:

                    value |= (
                        1 << (7 - bit)
                    )

            output.append(value)

    return output
###############################################################################
# Header generation
###############################################################################

def generate_header(
    name: str,
    width: int,
    height: int
) -> str:
    """
    Generate bitmap header.
    """

    guard = (
        f"{name.upper()}_H"
    )

    text = f"""\
#ifndef {guard}
#define {guard}

#include "bitmap_renderer.h"

extern const Bitmap_t {resource_name(name, width, height)};

#endif
"""

    return text
###############################################################################
# Source formatting
###############################################################################

def format_bitmap_data(data) -> str:
    """
    Format bitmap byte array.
    """

    text = ""

    for index, value in enumerate(data):

        if index % 12 == 0:
            text += "    "

        text += f"0x{value:02X}, "

        if index % 12 == 11:
            text += "\n"

    if not text.endswith("\n"):

        text += "\n"

    return text
###############################################################################
# Source generation
###############################################################################

def generate_source(
    name: str,
    width: int,
    height: int,
    data
) -> str:
    """
    Generate bitmap source.
    """

    bitmap = format_bitmap_data(data)

    bitmap_array = (
        f"{name}Bitmap"
    )

    text = f"""\
#include "{header_filename(name)}"

static const uint8_t {bitmap_array}[] =
{{
{bitmap}}};

const Bitmap_t {resource_name(name, width, height)} =
{{
    .width = {width},
    .height = {height},
    .bitmap = {bitmap_array}
}};
"""

    return text
###############################################################################
# File generation
###############################################################################

def write_file(
    path: Path,
    text: str
):
    """
    Write text file.
    """

    path.write_text(
        text,
        encoding="utf-8"
    )
###############################################################################
# PNG conversion
###############################################################################

def convert_png(
    path: Path,
    target_width: int,
    target_height: int,
    output_name: str | None = None,
    verbose: bool = False,
    remove_background: bool = True,
    bg_color: tuple[int, int, int] | None = None,
    bg_tolerance: int = BG_TOLERANCE
):
    """
    Convert one PNG image into Bitmap_t resource.
    """

    print(
        f"Converting {path.name} "
        f"-> {target_width}x{target_height}..."
    )

    if remove_background:
        if bg_color is not None:
            color_text = (
                f"#{bg_color[0]:02X}"
                f"{bg_color[1]:02X}"
                f"{bg_color[2]:02X}"
            )
        else:
            color_text = "auto"

        print(
            f"  Removing background: "
            f"{color_text}, tolerance={bg_tolerance}"
        )

    name = (
        output_name
        if output_name is not None
        else path.stem.lower()
    )

    bitmap, width, height = load_image(
        path,
        target_width,
        target_height,
        remove_background=remove_background,
        bg_color=bg_color,
        bg_tolerance=bg_tolerance
    )

    if verbose:
        print()

        for row in bitmap:
            print(row)

        print()

    packed = pack_bitmap(bitmap)

    header = generate_header(
        name,
        width,
        height
    )

    source = generate_source(
        name,
        width,
        height,
        packed
    )

    write_file(
        OUTPUT_DIR / header_filename(name),
        header
    )

    write_file(
        OUTPUT_DIR / source_filename(name),
        source
    )

    print(
        f"  -> {header_filename(name)}"
    )

    print(
        f"  -> {source_filename(name)}"
    )
###############################################################################
# CLI
###############################################################################

def parse_args():
    """
    Parse command-line arguments.
    """

    parser = argparse.ArgumentParser(
        description=(
            "Convert PNG images into Bitmap_t resources. "
            "Transparent borders are cropped before scaling."
        )
    )

    parser.add_argument(
        "input",
        type=Path,
        help=(
            "PNG file name or path. "
            f"Short names are resolved from {INPUT_DIR}, "
            "for example: humidity or humidity.png"
        )
    )

    parser.add_argument(
        "-w",
        "--width",
        type=int,
        required=True,
        help="Target bitmap width in pixels"
    )

    parser.add_argument(
        "-H",
        "--height",
        type=int,
        required=True,
        help="Target bitmap height in pixels"
    )

    parser.add_argument(
        "-o",
        "--output-name",
        type=str,
        help=(
            "Output resource base name. "
            "Defaults to the input file name."
        )
    )

    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Print generated bitmap rows"
    )

    parser.add_argument(
        "--remove-bg",
        action=argparse.BooleanOptionalAction,
        default=True,
        help=(
            "Remove solid background and make it transparent "
            "(default: enabled)"
        )
    )

    parser.add_argument(
        "--bg-color",
        type=str,
        help=(
            "Background color to remove: white, black, "
            "R,G,B or #RRGGBB. Auto-detected if omitted."
        )
    )

    parser.add_argument(
        "--bg-tolerance",
        type=int,
        default=BG_TOLERANCE,
        help=(
            "Background color matching tolerance "
            f"(0-255, default: {BG_TOLERANCE})"
        )
    )

    return parser.parse_args()


def resolve_input_path(input_path: Path) -> Path:
    """
    Resolve CLI input to an existing PNG file.

    Accepts:
    - full/relative path: bitmap_input/humidity.png
    - short name: humidity or humidity.png (looked up in bitmap_input)
    """

    candidates = []

    if input_path.is_absolute() or input_path.parent != Path("."):
        candidates.append(input_path)
    else:
        name = input_path.name

        if not name.lower().endswith(".png"):
            name = f"{name}.png"

        candidates.append(INPUT_DIR / name)
        candidates.append(input_path.with_suffix(".png"))

    for candidate in candidates:
        if candidate.exists():
            if candidate.suffix.lower() != ".png":
                raise ValueError(
                    f"Input file must be a PNG: {candidate}"
                )

            return candidate

    searched = ", ".join(
        str(path)
        for path in candidates
    )

    raise FileNotFoundError(
        f"Input file not found. Checked: {searched}"
    )


def collect_input_files(input_path: Path) -> list[Path]:
    """
    Resolve one input PNG file from CLI arguments.
    """

    return [resolve_input_path(input_path)]


###############################################################################
# Main
###############################################################################

def main():
    """
    Application entry point.
    """

    args = parse_args()

    if args.width <= 0 or args.height <= 0:
        print("Width and height must be positive integers.")
        return

    if not 0 <= args.bg_tolerance <= 255:
        print("Background tolerance must be in range 0..255.")
        return

    bg_color = None

    if args.bg_color is not None:
        try:
            bg_color = parse_color(args.bg_color)

        except ValueError as error:
            print(error)
            return

    ensure_output_directory()

    try:
        files = collect_input_files(args.input)

    except (FileNotFoundError, ValueError) as error:
        print(error)
        return

    converted = 0

    for path in files:

        try:
            convert_png(
                path,
                args.width,
                args.height,
                output_name=args.output_name,
                verbose=args.verbose,
                remove_background=args.remove_bg,
                bg_color=bg_color,
                bg_tolerance=args.bg_tolerance
            )

            converted += 1

        except Exception as error:

            print(
                f"ERROR: {path.name}"
            )

            print(
                error
            )

            print()

    print("----------------------------------------")
    print(f"Generated : {converted}")
    print(f"Output    : {OUTPUT_DIR}")
    print(f"Size      : {args.width}x{args.height}")
    print("----------------------------------------")


if __name__ == "__main__":
    main()