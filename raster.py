#!/usr/bin/python
import sys, os, time
import glob, json, argparse, copy
from PIL import Image

VERSION = "0.1"

### Setup Argument Parser
argparser = argparse.ArgumentParser(description='LasaurGrbl Raster GCode Generator', prog='lasauraster')
argparser.add_argument('image_file', metavar='image_file', nargs='?', default=False, help='Image file to raster')
argparser.add_argument('-v', '--version', action='version', version='%(prog)s ' + VERSION)
argparser.add_argument('-w', '--width',  dest='width_str', default="50", help='Width of the rastered image (mm)')
argparser.add_argument('-i', '--invert', dest='invert', action='store_true', default=False, help='Invert the image output')
argparser.add_argument('-o', '--out', 	 dest='outfile', default="out.gcode", help='destination file')
args = argparser.parse_args()


print args

print "Opening image file: " + args.image_file

target_width = float(args.width_str)

im = Image.open(args.image_file)
converted = im.convert("1")
converted.show()

# G8 P0.1
# G8 X50
# G8 N
# G8 D<data>
# G8 D<raster data encoded in ascii>
# G8 N
# G8 D<data>

count = 0
size = converted.size
width = float(size[0])
dot_size = target_width / width
target_height = float(size[1]) * dot_size

print "Dimensions: %.0fmm x %.0fmm, dot size = %f" % (target_width, target_height,  dot_size)

fw = file(args.outfile, "w")

fw.write("G8 P%.4f\n" % (dot_size))
fw.write("G8 X5\n")
fw.write("G8 N0\n")
string="G8 D"
for pixel in converted.getdata():
    if (args.invert):
	if (pixel > 128): pixel = '1'
	else: pixel = '0'
    else:
	if (pixel > 128): pixel = '0'
	else: pixel = '1'

    count=count+1
    
    string = string + pixel
    if (count % 35 == 34):
	fw.write(string + "\n")
	string="G8 D"

    if (count == width):
	fw.write(string + "\n")
	fw.write("G8 N0" + "\n")
	string="G8 D"
	count=0
	
fw.write(string + "\n")
fw.write("G8 N0\n")
fw.write("G0X0Y0\n")

fw.close()
