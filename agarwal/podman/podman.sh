# !/bin/bash
set -ev
cd "$(dirname "${BASH_SOURCE[0]}")"
podman build -t trajimage .
if [ $# -eq 0 ]; then
    # no args -> interactive mode
    podman run --rm -it --name=trajig -v $(pwd)/..:/me:z -w /me trajimage
else
    # args -> run one command, then exit
    # can use the second argument to set a timeout.
    echo "Running command: $1"
    echo "Timeout: ${2:-0}s"
    podman run --rm -it --timeout="${2:-0}" -v $(pwd)/..:/me:z -w /me trajimage $1
fi

