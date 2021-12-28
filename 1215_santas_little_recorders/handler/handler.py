#!/usr/bin/env python3

from PIL import Image

import sys, requests, eventlet, io, subprocess
import pprint

eventlet.monkey_patch()

TIMEOUT = 3
RECORDCMD = "/opt/record_description"

bannedchars = "`&!|#"

def handleInput(url):
    try:
        with eventlet.Timeout(TIMEOUT):
            r = requests.get(url)
            if r == None:
                return "Couldn't fetch URL"

            image = Image.open(io.BytesIO(r.content))
            if image == None:
                return "Doesn't look like an image"

            exifdata = image._getexif()
            if exifdata == None:
                return "There is no exif data"

            description = ""

            if 34853 in exifdata and 1 in exifdata[34853]:
                if exifdata[34853][1] != 'N':
                    return "Fail! That is not the north pole..."
            else:
                return "Fail! No GPS Latitude found"

            if 34853 in exifdata and 5 in exifdata[34853]:
                if exifdata[34853][5] != b'\x00':
                    return "Fail! That is not above sea level..."
            else:
                return "Fail! Not sure whether this is above or below ground..."

            if 270 in exifdata:
                if "santa" not in exifdata[270].lower():
                    return "Fail! Santa is not in this picture"
                else:
                    description = exifdata[270]
            else:
                return "Fail! This image has no description"

            if any([(x in bannedchars) for x in description]):
                return "Description has illegal characters"

            p = subprocess.Popen("{} {}".format(RECORDCMD, description), shell=True, encoding="utf-8", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output, stderr = p.communicate("")
            return output.strip()
            ret = "You wrote [{}]".format(url)

    except eventlet.timeout.Timeout:
        return "Fail! Processing took too long."
    except Exception as e:
        return "Something went wrong: {}".format(e)

print(handleInput(input().strip()))
sys.exit(0)

