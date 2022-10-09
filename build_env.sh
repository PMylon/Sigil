#!/bin/sh
# Initializes or activates the build environment
# Usage: source build_env.sh <mode>
# where mode can be one of the following:
# init       Initializes build environment, installing Zephyr's build meta tool west and additional python dependencies
# activate   Assumes that the build environment is already initialized and just activates it
# deactivate Assumes that the build environment is already activated and just deactivates it
# help       Prints usage information

zephyr_workspace="thirdparty/zephyrproject"
venv_dir="$zephyr_workspace/.venv"

init_env()
{
    cwd=$(pwd)

    # Create a pristine python virtual environment to avoid package incompatibilities at system or user level
    # First deactivate any prior activated virtual environment
    if [ -d "$venv_dir" ]; then
        deactivate_env
    fi
    python3 -m venv "$venv_dir" --clear

    source "$venv_dir/bin/activate"

    # Install Zephyr's meta tool west
    pip install west

    # Clone zephyr's source code and external modules
    west init -l "$zephyr_workspace/manifest"
    cd thirdparty/zephyrproject
    west update

    # Install additional Python dependencies
    pip install -r "zephyr/scripts/requirements.txt"

    # Install a toolchain
    cd $cwd
    source install_toolchain.sh

    cd $cwd

    # Update submodule for BOSSA bootloader
    git submodule update --init --recursive thirdparty/tools/BOSSA

    activate_env
}

activate_env()
{
    if [ ! -d "$venv_dir" ]; then
	    echo "error: Build environment is not initialized, check usage:"
        usage
    else
        echo "Activating build environment.."
        source "$venv_dir/bin/activate"
        export ZEPHYR_BASE=thirdparty/zephyrproject/zephyr
        echo "Build environment was activated!"
    fi
}

build()
{
    virtual_env=$(realpath $venv_dir)

    if [[ "$VIRTUAL_ENV" != $virtual_env ]]; then
        activate_env
    fi

    echo "Building project.."

    # cd thirdparty/tools/BOSSA
    # make
    # cd ../../../

    # west build -p auto -b arduino_nano_33_ble .
    west build -p auto -b frdm_k64f .
}

flash()
{
    virtual_env=$(realpath $venv_dir)

    if [[ "$VIRTUAL_ENV" != $virtual_env ]]; then
        activate_env
    fi

    echo "Flashing project"

    # sudo chmod a+rw /dev/ttyACM0

    # west flash --bossac=thirdparty/tools/BOSSA/bin/bossac
    west flash
}

debug()
{
    virtual_env=$(realpath $venv_dir)

    if [[ "$VIRTUAL_ENV" != $virtual_env ]]; then
        activate_env
    fi

    echo "Debugging.."

    west debug
}

deactivate_env()
{
    echo "Deactivating build environment.."
    deactivate
    echo "Build environment was deactivated!"
}

usage()
{
    name=`basename "${BASH_SOURCE[0]}"`

    cat << EOF
    usage: source $name <mode>

    Where mode can be one of the following:
    init       Initializes build environment, installing Zephyr's build meta tool west and additional python dependencies
    activate   Activates the build environment
    build      Builds the project. Activates the build environment if it is not already activated
    flash      Flash the project to the physical board
    debug      Debug project on the physical board
    deactivate Deactivates the build environment
    help       Prints usage information
EOF
}

case "$1" in
	"init")
		init_env
		;;

	"activate")
		activate_env
		;;

    "build")
        build
        ;;

    "flash")
        flash
        ;;

    "debug")
        debug
        ;;

    "deactivate")
        deactivate_env
        ;;

    "help")
        usage
        ;;
	*)
		echo "error: Wrong parameter, check usage:"
        usage
		;;
esac
