#!/usr/bin/env python3
# See chat for description.

from pathlib import Path
from PIL import Image

ICON_SIZE=32
THRESHOLD=128
INPUT_DIR=Path("source")
OUTPUT_DIR=Path("generated")
AUTO_CROP=True
CENTER_IMAGE=True
MASK_SOURCE="alpha"

def camel_case(name):
    return "".join(p.capitalize() for p in name.replace("-","_").split("_"))

def load_bitmap(path):
    img=Image.open(path).convert("RGBA")
    if AUTO_CROP:
        b=img.getbbox()
        if b: img=img.crop(b)
    img.thumbnail((ICON_SIZE,ICON_SIZE), Image.Resampling.LANCZOS)
    canvas=Image.new("RGBA",(ICON_SIZE,ICON_SIZE),(0,0,0,0))
    ox=(ICON_SIZE-img.width)//2 if CENTER_IMAGE else 0
    oy=(ICON_SIZE-img.height)//2 if CENTER_IMAGE else 0
    canvas.paste(img,(ox,oy),img)
    bm=[]
    for y in range(ICON_SIZE):
        row=[]
        for x in range(ICON_SIZE):
            r,g,b,a=canvas.getpixel((x,y))
            if MASK_SOURCE=="alpha":
                row.append(1 if a>=THRESHOLD else 0)
            else:
                gray=(299*r+587*g+114*b)//1000
                row.append(1 if gray<THRESHOLD else 0)
        bm.append(row)
    return bm

def pack_bits(bm):
    out=[]
    for row in bm:
        for s in range(0,ICON_SIZE,8):
            v=0
            for bit in range(8):
                x=s+bit
                if x<ICON_SIZE and row[x]:
                    v|=1<<(7-bit)
            out.append(v)
    return out

def write_header(name):
    guard=f"{name.upper()}_{ICON_SIZE}_H"
    cls=f"Icon{camel_case(name)}{ICON_SIZE}"
    txt=f"""#ifndef {guard}
#define {guard}
#include "icon_renderer.h"
#define {name.upper()}_ICON_WIDTH {ICON_SIZE}U
#define {name.upper()}_ICON_HEIGHT {ICON_SIZE}U
extern const Icon_t {cls};
#endif
"""
    (OUTPUT_DIR/f"{name}_{ICON_SIZE}.h").write_text(txt)

def write_source(name,data):
    cls=f"Icon{camel_case(name)}{ICON_SIZE}"
    txt=f'#include "{name}_{ICON_SIZE}.h"\n\nstatic const uint8_t icon_{name}_{ICON_SIZE}_bitmap[]={{\n'
    for i,b in enumerate(data):
        if i%12==0: txt+="    "
        txt+=f"0x{b:02X}, "
        if i%12==11: txt+="\n"
    txt+=f"\n}};\n\nconst Icon_t {cls}={{\n    .width={name.upper()}_ICON_WIDTH,\n    .height={name.upper()}_ICON_HEIGHT,\n    .bitmap=icon_{name}_{ICON_SIZE}_bitmap\n}};\n"
    (OUTPUT_DIR/f"{name}_{ICON_SIZE}.c").write_text(txt)

def main():
    OUTPUT_DIR.mkdir(exist_ok=True)
    files=sorted(INPUT_DIR.glob("*.png"))
    if not files:
        print("No PNG files in source/")
        return
    for f in files:
        p=pack_bits(load_bitmap(f))
        n=f.stem.lower()
        write_header(n)
        write_source(n,p)
        print("Generated",n)
if __name__=="__main__":
    main()
