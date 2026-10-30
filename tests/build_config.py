"""Update the build stamp only when effective build flags change."""
import json
from pathlib import Path
import sys
path = Path(sys.argv[1])
content = json.dumps(sys.argv[2:])
if not path.exists() or path.read_text() != content:
    path.write_text(content)
