import sys
from xml.dom import minidom
from dataclasses import dataclass
from copy import deepcopy
from svgpathtools import parse_path, Line, QuadraticBezier, CubicBezier


def get_color(string: str):
    assert string[0] == '#'
    string = string[1:]
    length = len(string)

    result = []
    if length == 3:
        result = [int(string[i : i+1], 16)*17/255 for i in range(3)]
    elif length == 6:
        result = [int(string[2*i : 2*(i+1)], 16)/255 for i in range(3)]
    else:
        assert False

    result.append(1.0)
    return result

@dataclass
class Context:
    fill: str = None
    stroke: str = None
    stroke_width: str = "1.0"

doc = minidom.parse(sys.argv[1])

svg = doc.getElementsByTagName("svg")[0]

all_keys = set()


print("auto tiger = List<Tiger_Path>{")

def process(node, context: Context):
    if node.nodeType != node.ELEMENT_NODE:
        return

    my_context = deepcopy(context)

    kind = node.tagName
    fill = node.getAttribute("fill")
    stroke = node.getAttribute("stroke") 
    stroke_width = node.getAttribute("stroke-width") 

    for key in node.attributes.keys():
        all_keys.add(key)

    if fill == "none": fill = None
    if stroke == "none": stroke = None

    if fill != "":          my_context.fill = fill
    if stroke != "":        my_context.stroke = stroke
    if stroke_width != "":  my_context.stroke_width = stroke_width

    if kind == "path":
        path = parse_path(node.getAttribute("d"))

        print("{")
        print("    {")

        print("        List<Generic_Bezier>{")
        for curve in path:
            print("            ", end = "")
            if isinstance(curve, Line):
                print("generic_line(", end = "")
            if isinstance(curve, QuadraticBezier):
                print("generic_quadratic(", end = "")
            if isinstance(curve, CubicBezier):
                print("generic_cubic(", end = "")

            print(", ".join([ f"{{{point.real}f, {point.imag}f}}" for point in curve]), end = "")
            print("),")

        print("        },")

        print(f"        {str(path.isclosed()).lower()},")
        print("    },")

        fill_color   = (0.0, 0.0, 0.0, 0.0)
        stroke_color = (0.0, 0.0, 0.0, 0.0)
        stroke_width = 0.0

        if my_context.fill is not None:
            fill_color = get_color(my_context.fill)

        if my_context.stroke is not None:
            stroke_color = get_color(my_context.stroke)
            stroke_width = my_context.stroke_width

        if my_context.stroke_width is not None:
            stroke_width = float(my_context.stroke_width)

        print(f"    {{ {fill_color[0]}f, {fill_color[1]}f, {fill_color[2]}f, {fill_color[3]}f }},")
        print(f"    {{ {stroke_color[0]}f, {stroke_color[1]}f, {stroke_color[2]}f, {stroke_color[3]}f }},")
        print(f"    {stroke_width}f,")

        
        print("}, ")

    for child in node.childNodes:
        process(child, my_context)

process(svg, Context())

print("};")
