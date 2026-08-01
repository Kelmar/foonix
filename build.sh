#!/bin/sh
set -e

cd "$(dirname "$0")"

usage() {
    echo "Usage: $0 [platform] [-o|--output <file>]"
    echo ""
    echo "Available platforms:"
    for f in src/config/*.json; do
        name=$(basename "$f" .json)
        [ "$name" = "lint" ] && continue
        echo "  $name"
    done
    echo ""
    echo "  -o, --output <file>  ISO output filename, relative to ./out/"
    echo "                       (default: boot.<platform>.iso)"
    exit 1
}

PLATFORM=""
OUTPUT=""

while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            usage
            ;;
        -o|--output)
            OUTPUT="$2"
            shift 2
            ;;
        *)
            if [ -n "$PLATFORM" ]; then
                echo "Unexpected argument: $1" >&2
                echo "" >&2
                usage
            fi
            PLATFORM="$1"
            shift
            ;;
    esac
done

PLATFORM="${PLATFORM:-x86_64}"
OUTPUT="${OUTPUT:-boot.${PLATFORM}.iso}"

if [ ! -f "src/config/${PLATFORM}.json" ] || [ "$PLATFORM" = "lint" ]; then
    echo "Unknown platform: $PLATFORM" >&2
    echo "" >&2
    usage
fi

docker build -f docker/Dockerfile.tools -t foonix-tools:latest --load .

docker build \
    -f docker/Dockerfile.build \
     --progress=plain \
     --build-arg CONFIG="$PLATFORM" \
     --build-arg OUTPUT="$OUTPUT" \
     -t \
     foonix-env:latest \
     -o type=local,dest=./out .

echo "Build Complete"
