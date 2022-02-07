# Converter for generating presets from CTP DICOM Anonymizer settings
import sys
import os

if __name__ == "__main__":
    with open('input.txt') as f:
        # Using for loop
        for line in f:            
            if not line.strip():
                continue
            first_id = line[13:17]
            second_id = line[17:21]

            command = "INVALID"

            if "></e>" in line:
                command="safe"
            if ">@empty()</e>" in line:
                command="clear"
            if ">@remove()</e>" in line:
                command="remove"

            out_line = "res &= addTag(\"(" + first_id + "," + second_id + ")\", \""+command+"\");"
            print(out_line)
