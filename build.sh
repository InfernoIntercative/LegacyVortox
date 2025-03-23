#!/bin/bash

clear

if make; then
  ./build/bin/Vortox "$@"
else
  echo "Compilation failed. The binary will not be executed."
  exit 1
fi
