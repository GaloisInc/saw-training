import os
import os.path
from pathlib import Path
import re

# Filenames that match this regex will be included in the downloads directory
file_pattern = r'^([a-zA-Z0-9_-]+\.(c|cry|h|saw)|Makefile)$'

# Comments that match this regex will not be included in the files in the downloads directory
comment_pattern = r'^\s*(//|#)\s+(BEGIN|END).+$'

# Skip things like editor autosave files and build products
def use_file(filename):
    return True if re.match(file_pattern, filename) else False

def generate_downloads(start, end):
    for (this_dir, sub_dirs, files) in os.walk(start):
        for f in files:
            if use_file(f):
                in_file = os.path.join(this_dir, f)
                out_file = os.path.join(end, in_file)
                with open(in_file) as the_file:
                    out_contents = \
                        ''.join(line
                                for line
                                in the_file
                                # Delete file-inclusion markers
                                if not re.match(comment_pattern, line))

                # Ensure that the target directory exists
                Path(os.path.dirname(out_file)).mkdir(parents=True, exist_ok=True)

                with open(out_file, "w") as out:
                    out.write(out_contents)

if __name__ == "__main__":
    generate_downloads("examples", "downloads")
