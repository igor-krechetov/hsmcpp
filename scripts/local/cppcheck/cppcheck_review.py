#!/usr/bin/python3
import sys
import xml.etree.ElementTree as ET


if __name__ == "__main__":
    REPORT_FILE = sys.argv[1].strip(" \n\r[]")
    print(f"{REPORT_FILE=:}")
    tree = ET.parse(REPORT_FILE)

    if tree:
        root = tree.getroot()
        errors = root.find("errors")
        issuesCount = 0

        for currentError in errors.iter("error"):
            # NOTE: there is a crash in misra.py. fixed in 2.8+ (b444c00)
            #       misra plugin couldn't handle passing string literal to a function with variadic arguments
            if ("id" not in currentError.attrib) or (currentError.attrib["id"] != "internalError"):
                issuesCount += 1
                ET.dump(currentError)

        if issuesCount > 0:
            print("cppcheck: FOUND ISSUES")
            exit(1)
        else:
            print("cppcheck: no issues found")
