#!/usr/bin/env bash
set -Eeo pipefail
echo "-- Starting anonymizer..."
./mercure-anonymizer $MERCURE_IN_DIR $MERCURE_OUT_DIR
echo "-- Done."
