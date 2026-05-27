from fontTools.ttLib import TTFont
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.pens.pointPen import PointToSegmentPen
from fontTools.pens.recordingPen import RecordingPen

from pathlib import Path
import turtle


def to_c_list(py_list, linebreak=False, default="{}"):
    if type(py_list) == list:
        if len(py_list) == 0:
            return default
        
        if linebreak:
            return "{" + ",\n".join(map(lambda e: to_c_list(e, False, default), py_list)) + "}"
        else:
            return "{" + ",".join(map(lambda e: to_c_list(e, False, default), py_list)) + "}"
    
    return str(py_list)


def to_py_list(py_list, linebreak=False, default="[]"):
    if type(py_list) == list:
        if len(py_list) == 0:
            return default
        if linebreak:
            return "[" + ",\n".join(map(lambda e: to_py_list(e, False, default), py_list)) + "]"
        else:
            return "[" + ",".join(map(lambda e: to_py_list(e, False, default), py_list)) + "]"
    
    return str(py_list)


def convert_coords(coords, endPts, skip=1):
    new_coords = []
    new_endpts = []

    i = 0
    j = 0
    ep_i = 0

    while i < len(coords):
        if ep_i >= len(endPts):
            ep = len(coords) - 1
        else:
            ep = endPts[ep_i]

        new_coords.append(coords[i])

        j = i + 1
        i -= 1

        if j < ep:
            if coords[j] != coords[i]:
                i += 1

            while j < ep:
                if coords[j] == coords[i]:
                    j += 1
                    i -= 1
                else:
                    break
        if j == ep:
            new_endpts.append(len(new_coords)-1)
            ep_i += 1
            i = j + 1
        else:
            i = j

    return new_coords[::skip], [int(ep/skip) for ep in new_endpts]


def get_data(data):
    all_coords = [[[0, 0]] for _ in range(127)]
    all_endpts = [[0] for _ in range(127)]
    all_flags = [[] for _ in range(127)]
    all_hmtx = [[0, 0] for _ in range(127)]

    for char in "!""#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~":
        char_data = data.get(char)
        if not char_data:
            continue

        coords = char_data['coords_flat']
        endpts = list(map(len, char_data['contours']))
        flags = char_data['flags']
        hmtx = list(char_data['hmtx'].values())

        all_coords[ord(char)] = coords
        all_endpts[ord(char)] = endpts
        all_flags[ord(char)] = flags
        all_hmtx[ord(char)] = hmtx

    return all_coords, all_endpts, all_flags, all_hmtx


def get_glyph_data(font_path):
    font = TTFont(font_path)

    cmap = font.getBestCmap()
    glyph_set = font.getGlyphSet()
    hmtx = font["hmtx"].metrics

    results = {}

    for codepoint, glyph_name in cmap.items():
        glyph = glyph_set[glyph_name]

        # -------------------------
        # 1. hmtx data
        # -------------------------
        advance_width, lsb = hmtx.get(glyph_name, (None, None))

        # -------------------------
        # 2. outline recording
        # -------------------------
        rec = RecordingPen()
        glyph.draw(rec)

        coords = []
        contours = []
        flags = []  # only meaningful for TrueType outlines

        current_contour = []
        contour_end_indices = []

        point_index = 0

        for op, pts in rec.value:
            if op == "moveTo":
                if current_contour:
                    contours.append(current_contour)
                    current_contour = []

                current_contour.append(pts[0])
                flags.append(1)  # on-curve point (TrueType style assumption)
                point_index += 1

            elif op == "lineTo":
                current_contour.append(pts[0])
                flags.append(1)
                point_index += 1

            elif op == "qCurveTo" or op == "curveTo":
                # cubic/quadratic curves still represented as points here
                for p in pts:
                    current_contour.append(p)
                    flags.append(0)  # off-curve approximation
                    point_index += 1

            elif op == "closePath":
                if current_contour:
                    contours.append(current_contour)
                    current_contour = []

        if current_contour:
            contours.append(current_contour)

        results[chr(codepoint)] = {
            "glyph_name": glyph_name,
            "hmtx": {
                "advance_width": advance_width,
                "lsb": lsb,
            },
            "contours": contours,
            "coords_flat": [pt for contour in contours for pt in contour],
            "flags": flags,
        }

    return results


def write_files(all_coords, all_endpts, all_flags, all_hmtx, python_file, c_file):
    max_coords = max(map(len, all_coords))
    
    with open(c_file, mode='w', encoding='ascii') as f:
        f.write(f"int coordinates[127][{max_coords}][2] = ")
        f.write(to_c_list(all_coords, True, default="{{0,0}}"))
        f.write("\n\n")
        f.write("int endPts[127][3] = ")
        f.write(to_c_list(all_endpts, True, default="{0}"))
        f.write("\n\n")
        f.write("int hmtx[127][2] = ")
        f.write(to_c_list(all_hmtx, True, default="{{0,0}}"))


    with open(python_file, mode='w', encoding='ascii') as f:
        f.write("coordinates = ")
        f.write(to_py_list(all_coords, True, default="[[0,0]]"))
        f.write("\n\n")
        f.write("endPts = ")
        f.write(to_py_list(all_endpts, True, default="[0]"))
        f.write("\n\n")
        f.write("hmtx = ")
        f.write(to_py_list(all_hmtx, True, default="[[0,0]]"))


def turtle_demo():
    glyph = glyf["A"]  # glyph name (not always the literal character!)

    # Get coordinates, flags, and contour info
    coordinates, endPts, flags = glyph.getCoordinates(font["glyf"])

    print("Coordinates:", coordinates)
    print("End points of contours:", endPts)
    print("Flags:", flags)


    # Setup turtle
    t = turtle.Turtle()
    t.speed(0)
    turtle.setworldcoordinates(-500, -500, 1500, 1500)

    start = 0

    for end in endPts:
        contour = coordinates[start:end+1]
        contour = contour[:int(len(contour)/2)+1]

        # Move to first point
        first_point = contour[0]
        t.penup()
        t.goto(first_point[0], first_point[1])
        t.pendown()

        # Draw lines between points
        for point in contour[1:]:
            t.goto(point[0], point[1])

        # Close contour
        # t.goto(first_point[0], first_point[1])

        start = end + 1

    turtle.done()


if __name__ == "__main__":
    parent_path = Path(__file__).parent
    font_path = parent_path.joinpath("funky-signature-font/FunkySignature-Zmgz.otf")

    data = get_glyph_data(font_path)

    # # example output for 'A'
    # print(data.get("A"))

    all_coords, all_endpts, all_flags, all_hmtx = get_data(data)
    write_files(all_coords, all_endpts, all_flags, all_hmtx, parent_path.joinpath("fontdata_py.txt"), parent_path.joinpath("fontdata_c.txt"))

    print("done")

    # co = [[122,720],[121,650],[117,544],[112,443],[109,336],[109,266],[109,336],[112,443],[117,544],[121,650],[122,720],[103,234],[103,240],[110,240],[116,240],[116,234],[116,227],[110,227],[103,227]]
    # ep = [10,18]
    # print(co, ep)
    # co, ep = convert_coords(co, ep)
    # print(co, ep)