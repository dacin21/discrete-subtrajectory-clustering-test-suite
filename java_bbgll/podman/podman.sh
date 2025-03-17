# !/bin/bash
set -ev
cd "$(dirname "${BASH_SOURCE[0]}")"
podman build -t buchinimagetoo .
if [ $# -eq 0 ]; then
    # no args -> interactive mode
    podman run --rm -it --name=buchintoo -v $(pwd)/../../data:/data:z -v $(pwd)/..:/me:z -w /me buchinimagetoo
else
    # args -> run one command, then exit
    echo "Running command: $1"
    echo "Timeout: ${2:-inf }s"
    podman run --rm -it --timeout="${2:-0}" -v $(pwd)/../../data:/data:z -v $(pwd)/..:/me:z -w /me buchinimagetoo $1
fi

