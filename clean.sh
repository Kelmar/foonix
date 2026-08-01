#!/bin/sh

while getopts ":ht:" opt; do
  case ${opt} in
    h)
        echo "Cleans build output."
        echo "Options:"
        echo " -t -- Also clean build/tools folder"
        exit 0
        ;;

    t)
        rm -rf build/tools
        ;;
  esac
done

rm -rf out
