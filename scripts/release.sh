set -eo pipefail

# working path
HOME=$(cd "$(dirname "$0")"; pwd)
PROJECT="$HOME/.."

pushd "$PROJECT"
zip -r archives.zip . -x "*.zip" \
  -x ".gitmodules" \
  -x ".git/" -x ".git/*" \
  -x "libuv/" -x "libuv/*" \
  -x "build/" -x "build/*" \
  -x "bin/" -x "bin/*" \
  -x "cmake-build-*/" -x "cmake-build-*/*" \
  -x "scripts/logs/" -x "scripts/logs/*" \
  -x "scripts/*.json" \
  -x ".DS_Store"
popd
