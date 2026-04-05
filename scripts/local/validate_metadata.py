#!/usr/bin/python3
import sys
import re

def validateFile(filePath, pattern, expectedValue):
    isValid = False
    with open(filePath, "r") as f:
        for line in f:
            match = re.search(pattern, line)
            if match:
                value = match.group(1)
                isValid = (value == expectedValue)
                break
    if isValid == False:
        print(f"Failed validation. Please update library version in {filePath}")
    return isValid


if __name__ == "__main__":
    COMMIT_VERSION = sys.argv[1].strip(" \n\r[]")
    REPO_DIR = sys.argv[2]

    print(f"{COMMIT_VERSION=:}")
    print(f"{REPO_DIR=:}")

    # first validate commit version itself
    match = re.search(r"^(\d+\.\d+\.\d+)$", COMMIT_VERSION)
    if match is None:
        print(f"ERROR: invalid commit version format. Expected a.b.c, but got '{COMMIT_VERSION}'")
        exit(1)

    results = []
    results.append( validateFile(f"{REPO_DIR}/README.md", r"changelog-v(\d+\.\d+\.\d+)-", COMMIT_VERSION) )
    results.append( validateFile(f"{REPO_DIR}/CHANGELOG.md", r"## \[(\d+\.\d+\.\d+)\] - ", COMMIT_VERSION) )
    results.append( validateFile(f"{REPO_DIR}/CMakeLists.txt", r"set\(PROJECT_VERSION +\"(\d+\.\d+\.\d+)\"\)", COMMIT_VERSION) )

    if False in results:
        exit(2)
        