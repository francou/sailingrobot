#!/bin/sh
set -e

# check $0 in pwd
if [ ! -e "$(pwd)/$(basename "$0")" ]; then
    printf 'Spooky, I do not exist in %s ...\n' "$(pwd)"
    exit 1
fi
case "$1" in
    init)
        ;;
    update)
        git pull &&
            git submodule sync --recursive &&
            git submodule update --init --recursive &&
            printf '%s: Done\n' "$0"
                    ;;

    status)
        git status
        git submodule foreach "git status"
        ;;
    *)
        printf 'Usage: %s update|status\n' "$0"
        ;;
esac
