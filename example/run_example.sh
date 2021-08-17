#!/usr/bin/env bash
set -eo pipefail

copy_if_present() {
  if [ -a "$1" ]; then
    cp "$1" "$2"
  fi
}

PCEJS_ARCH=$1
if [[ -z $PCEJS_ARCH ]]
  then
    echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
    exit 1
fi

PCEJS_MODE=${2:-""}


PCEJS_DIR=$(git rev-parse --show-toplevel)

PCEJS_MODULES_DIR="$PCEJS_DIR/commonjs"
PCEJS_DIST_DIR="$PCEJS_DIR/dist"
PCEJS_ARCH_MODULE_DIR="$PCEJS_MODULES_DIR/pcejs-${PCEJS_ARCH}"
PCEJS_ARCH_EXAMPLE_DIR="$PCEJS_DIR/example/$PCEJS_ARCH"
PCEJS_NODE_BIN_DIR=$PCEJS_DIR/node_modules/.bin/

if ! (ls "${PCEJS_ARCH_EXAMPLE_DIR}"/*.rom >/dev/null 2>&1); then
  echo ""
  echo "you need to download the ${PCEJS_ARCH} system to run this example "
  echo ""
  echo "grab it from: "
  echo "https://jamesfriend.com.au/pce-js/dist/${PCEJS_ARCH}-system.zip"
  echo ""
  echo "and extract it into: "
  echo "$PCEJS_ARCH_EXAMPLE_DIR"
  echo ""
  exit 1
fi

"$PCEJS_ARCH_MODULE_DIR"/prepublish.sh $PCEJS_ARCH

NODE_PATH="$PCEJS_MODULES_DIR:$PCEJS_DIR/node_modules:"$(dirname $EMSDK_NODE)"/../lib/node_modules/" \
  "browserify" "$PCEJS_ARCH_EXAMPLE_DIR/$PCEJS_ARCH.js" \
    --noparse="$PCEJS_ARCH_MODULE_DIR/lib/pcejs-${PCEJS_ARCH}.js" \
    > "$PCEJS_ARCH_EXAMPLE_DIR/bundle.js"
js-beautify "$PCEJS_ARCH_EXAMPLE_DIR/bundle.js" | sed -s '/xhr.onload = function/ixhr.setRequestHeader("Cache-Control", "no-cache");' > "$PCEJS_ARCH_EXAMPLE_DIR/bundle.js.tmp"
mv "$PCEJS_ARCH_EXAMPLE_DIR/bundle.js.tmp" "$PCEJS_ARCH_EXAMPLE_DIR/bundle.js"
echo "bundle.js built"

if [[ $PCEJS_MODE == "worker" ]]; then
  {
    cat "${PCEJS_ARCH_EXAMPLE_DIR}/worker-prelude.js" "$PCEJS_ARCH_MODULE_DIR/pce-${PCEJS_ARCH}.worker.js" 
  } > "${PCEJS_ARCH_EXAMPLE_DIR}/pce-${PCEJS_ARCH}.worker.js"

  echo "built with worker"
elif [[ $PCEJS_MODE == "wasm" ]]; then
  echo "built with wasm"
elif [[ $PCEJS_MODE == "threaded" ]]; then
  {
    cat "${PCEJS_ARCH_EXAMPLE_DIR}/pthread-prelude.js" "${PCEJS_DIST_DIR}/pce-${PCEJS_ARCH}.js"
  } > "${PCEJS_ARCH_EXAMPLE_DIR}/bundle.js"
  cp "$PCEJS_ARCH_MODULE_DIR/pthread-main.js" "${PCEJS_ARCH_EXAMPLE_DIR}/pthread-main.js"

  echo "built with pthreads"
fi

# copy over wasm too
copy_if_present "$PCEJS_ARCH_MODULE_DIR/pce-${PCEJS_ARCH}.wasm" "${PCEJS_ARCH_EXAMPLE_DIR}/pce-${PCEJS_ARCH}.wasm"
copy_if_present "$PCEJS_ARCH_MODULE_DIR/pce-${PCEJS_ARCH}.js.mem" "${PCEJS_ARCH_EXAMPLE_DIR}/pce-${PCEJS_ARCH}.js.mem"
copy_if_present "$PCEJS_ARCH_MODULE_DIR/${PCEJS_ARCH}-pcex.rom" "${PCEJS_ARCH_EXAMPLE_DIR}/${PCEJS_ARCH}-pcex.rom"
 
# node "$PCEJS_DIR/example/webserver.js" "$PCEJS_DIR/example/$PCEJS_ARCH"
