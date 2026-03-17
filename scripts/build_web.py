"""
Pre-build script: Minify and gzip index.html into a C header for PROGMEM.
Runs before compilation starts.
"""
import gzip
import os
import re

Import("env")  # noqa: F821 - PlatformIO magic

def build_web_header(source, target, env):
    html_path = os.path.join(env["PROJECT_DIR"], "data", "web", "index.html")
    header_path = os.path.join(env["PROJECT_DIR"], "src", "web_content.h")

    if not os.path.exists(html_path):
        print(f"WARNING: {html_path} not found, skipping web build")
        return

    # Check if header is up-to-date
    if os.path.exists(header_path):
        if os.path.getmtime(header_path) > os.path.getmtime(html_path):
            print("web_content.h is up-to-date")
            return

    with open(html_path, "r", encoding="utf-8") as f:
        html = f.read()

    # Basic minification: remove HTML comments, strip JS single-line comments,
    # then collapse whitespace
    html = re.sub(r"<!--.*?-->", "", html, flags=re.DOTALL)
    # Remove JS single-line comments (// ...) but not URLs (://)
    html = re.sub(r"(?<!:)//[^\n]*", "", html)
    html = re.sub(r"\s+", " ", html)
    html = html.replace("> <", "><")

    # Gzip compress
    compressed = gzip.compress(html.encode("utf-8"), compresslevel=9)

    # Generate C header
    lines = []
    lines.append("#pragma once")
    lines.append("#include <Arduino.h>")
    lines.append("")
    lines.append(f"// Auto-generated from index.html ({len(compressed)} bytes gzipped)")
    lines.append(f"const size_t WEB_CONTENT_SIZE = {len(compressed)};")
    lines.append("const uint8_t WEB_CONTENT[] PROGMEM = {")

    # Format bytes as hex
    hex_lines = []
    for i in range(0, len(compressed), 16):
        chunk = compressed[i : i + 16]
        hex_str = ", ".join(f"0x{b:02x}" for b in chunk)
        hex_lines.append(f"    {hex_str},")
    lines.extend(hex_lines)

    lines.append("};")
    lines.append("")

    with open(header_path, "w") as f:
        f.write("\n".join(lines))

    print(f"Built web_content.h: {len(html.encode('utf-8'))} -> {len(compressed)} bytes")

# Run immediately when script is loaded (pre: scripts run before build)
build_web_header(None, None, env)
