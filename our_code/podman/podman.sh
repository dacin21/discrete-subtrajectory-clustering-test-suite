# !/bin/bash
set -ev
cd "$(dirname "${BASH_SOURCE[0]}")"
podman build -t clustering_image .

# !/bin/bash
if [ $# -eq 0 ]; then
    # no args -> interactive mode
    podman run --rm -it --name=clustering -v $(pwd)/..:/me:z -v $(pwd)/../../data:/data:z -w /me clustering_image
else
    # args -> run one command, then exit
    echo "Running command: $1"
    echo "Timeout: ${2:-inf }s"
    podman run --rm -it --timeout="${2:-0}" -v $(pwd)/..:/me:z -v $(pwd)/../../data:/data:z -w /me clustering_image $1
fi

