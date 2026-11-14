#!/usr/bin/env python3
# trunk-ignore-all(ruff/F821)
# trunk-ignore-all(flake8/F821): For SConstruct imports
"""Inject the version from version.properties into the build as APP_VERSION."""
import os
import sys

Import("env")

sys.path.append(os.path.join(env["PROJECT_DIR"], "bin"))
from readprops import readProps  # noqa: E402

verObj = readProps(os.path.join(env["PROJECT_DIR"], "version.properties"))
print(f"Using standalone-ui version {verObj['long']} on {env.get('PIOENV')}")

env.Append(
    CPPDEFINES=[
        ("APP_VERSION", env.StringifyMacro(verObj["long"])),
        ("APP_VERSION_SHORT", env.StringifyMacro(verObj["short"])),
    ]
)
