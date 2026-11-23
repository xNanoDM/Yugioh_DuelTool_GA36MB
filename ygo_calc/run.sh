#!/bin/sh
here=$(dirname "$(readlink -f "$0")")
export LD_LIBRARY_PATH="$here/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export SDL_VIDEODRIVER=kmsdrm
export SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS=1
chmod +x "$here/ygo_calc" 2>/dev/null || true
exec "$here/ygo_calc" "$@" > "$here/run.log" 2>&1
