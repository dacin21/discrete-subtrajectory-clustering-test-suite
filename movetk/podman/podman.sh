# !/bin/bash
set -ev
cd "$(dirname "${BASH_SOURCE[0]}")"
podman build -t movetkimage .

if [ $# -eq 0 ]; then
    # no args -> interactive mode
    podman run --rm -it --name=movetk -v $(pwd)/..:/me:Z -v $(pwd)/../../data:/data:Z -w /me movetkimage
else
    # args -> run one command, then exit
    echo "Running command: $1"
    podman run --rm --memory=115g --memory-swap=115g -it -v $(pwd)/..:/me:Z -v $(pwd)/../../data:/data:Z -w /me movetkimage $1
fi

