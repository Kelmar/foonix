#!/bin/sh
set -e

cd "$(dirname "$0")"

usage() {
    echo "Usage: $0 [platform]"
    echo ""
    echo "Available platforms:"
    for f in src/config/*.json; do
        name=$(basename "$f" .json)
        [ "$name" = "lint" ] && continue
        echo "  $name"
    done
    exit 1
}

case "$1" in
    -h|--help)
        usage
        ;;
esac

PLATFORM="${1:-x86_64}"

if [ ! -f "src/config/${PLATFORM}.json" ] || [ "$PLATFORM" = "lint" ]; then
    echo "Unknown platform: $PLATFORM" >&2
    echo "" >&2
    usage
fi

docker build \
    -f docker/Dockerfile.build \
     --progress=plain \
     --build-arg CONFIG="$PLATFORM" \
     -t \
     foonix-env:latest \
     -o type=local,dest=./out .
