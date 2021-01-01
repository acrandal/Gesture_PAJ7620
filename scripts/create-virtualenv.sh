#!/bin/bash

WARNING_COLOR='\033[1;33m'
ERROR_COLOR='\033[1;31m'
RESET_COLOR='\033[0m'

VIRTUALENV_NAME=".paj7620-env"

function print_warning() {
  echo -e "${WARNING_COLOR}WARNING: $1${RESET_COLOR}"
}

function print_error() {
  echo -e "${ERROR_COLOR}ERROR: $1${RESET_COLOR}"
}

# Verify that python3 and virtualenv are installed
if ! which python3 &> /dev/null; then
  print_error "Could not find python3, please ensure it is installed."
  exit 1
fi

if ! which virtualenv &> /dev/null; then
  print_error "Could not find virtualenv, please ensure it is installed (e.g pip3 install virtualenv)"
  exit 1
fi

if [[ -d "${VIRTUALENV_NAME}" ]]; then
  # Environment already exists. Confirm recreation.
  read -rp "Virtual environment already exists, would you like to recreate it? [y/N] " confirmation
  if [[ "${confirmation,,}" =~ ^(yes|y)$ ]]; then
    # Ensure environment not currently active
    deactivate &> /dev/null
    rm -rf "${VIRTUALENV_NAME}"
  else
    print_warning "Environment already exists, exiting..."
    exit 2
  fi
fi

# Setup virtual environment
virtualenv "${VIRTUALENV_NAME}"

echo "Virtual environment created, performing first time setup..."

# Activate and perform first time requirements install
source "${VIRTUALENV_NAME}/bin/activate"
pip install -r requirements.txt

echo "Setup complete. Activate the virtual environment with 'source ${VIRTUALENV_NAME}/bin/activate'."
