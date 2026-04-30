#!/bin/bash
PROJ="app"
CLEAN_TYPE=0
shopt -s nocasematch
while getopts ":p:c" arg; do  
    case $arg in 
        c)
            CLEAN_TYPE=1
            ;;
        p)
            PROJ=${OPTARG}
            ;;
    esac 
done 
shift $[$OPTIND - 1]

export SENSOR_ELF_OUTPUT_PATH="./bin"
export DEMO_APP_OUTPUT_PATH="./src/$PROJ/build"
export BUILD_LOG_FILE="build.log"
export Local_MAKE="/opt/QualComm/make3.81/bin"

if [ ! -d "$SENSOR_ELF_OUTPUT_PATH" ];then
  mkdir "$SENSOR_ELF_OUTPUT_PATH"
fi

if [ ! -d "$DEMO_APP_OUTPUT_PATH" ];then
  mkdir "$DEMO_APP_OUTPUT_PATH"
fi

if [ -f "$BUILD_LOG_FILE" ];then
  rm "$BUILD_LOG_FILE"
fi

if [ $CLEAN_TYPE -eq 1 ]
  then
    echo "Cleaning..."
    rm -rf $SENSOR_ELF_OUTPUT_PATH/*
    rm -rf $DEMO_APP_OUTPUT_PATH/*
    echo "Done."
    exit
fi

$Local_MAKE/make all OS=LINUX COMP=LLVM PROJ=$PROJ
