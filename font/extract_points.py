from fontTools.ttLib import TTFont
from pathlib import Path
import turtle


def to_c_list(py_list, linebreak=False):
    if type(py_list[0]) == list:
        if linebreak:
            return "{" + ",\n".join(map(to_c_list, py_list)) + "}"
        else:
            return "{" + ",".join(map(to_c_list, py_list)) + "}"
    
    return "{" + ",".join(map(str, py_list)) + "}"


parent_path = Path(__file__).parent
font = TTFont(parent_path.joinpath("FlightySingleLine-0W934.ttf"))

glyf = font["glyf"]

glyph_names = font.getGlyphOrder()

all_coords = [[0, 0] for _ in range(ord('z')+1)]
all_endpts = [[0] for _ in range(ord('z')+1)]
all_flags = [[] for _ in range(ord('z')+1)]

for name in glyph_names:
    if len(name) == 1:
        print(name, ord(name))
        glyph = glyf[name]
        coordinates, endPts, flags = glyph.getCoordinates(font["glyf"])

        all_coords[ord(name)] = [[co[0], co[1]] for co in coordinates]
        all_endpts[ord(name)] = endPts
        all_flags[ord(name)] = flags

print(all_coords)

with open(parent_path.joinpath("fontdata.txt"), mode='w', encoding='ascii') as f:
    f.write("int coordinates[] = ")
    f.write(to_c_list(all_coords, True))
    f.write("\n\n")
    f.write("int endPts[] = ")
    f.write(to_c_list(all_endpts))
    

glyph = glyf["A"]  # glyph name (not always the literal character!)

# Get coordinates, flags, and contour info
coordinates, endPts, flags = glyph.getCoordinates(font["glyf"])

# print("Coordinates:", coordinates)
# print("End points of contours:", endPts)
# print("Flags:", flags)




# # Setup turtle
# t = turtle.Turtle()
# t.speed(0)
# turtle.setworldcoordinates(-500, -500, 1500, 1500)

# start = 0

# for end in endPts:
#     contour = coordinates[start:end+1]
#     contour = contour[:int(len(contour)/2)+1]

#     # Move to first point
#     first_point = contour[0]
#     t.penup()
#     t.goto(first_point[0], first_point[1])
#     t.pendown()

#     # Draw lines between points
#     for point in contour[1:]:
#         t.goto(point[0], point[1])

#     # Close contour
#     # t.goto(first_point[0], first_point[1])

#     start = end + 1

# turtle.done()