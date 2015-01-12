#ifndef OPTIONS_H
#define OPTIONS_H

/*******************************************************************************
 *
 * This file defines several command line options used by main
 *
 * @file Options.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <optionparser.h>

/******************************************************************************/

enum OptionIndex { 
     UNKNOWN
    ,HELP
    ,RUN_TESTS_AND_QUIT
    ,PRINT_CONFIG_AND_QUIT
    ,PRINT_CAMERA
    ,DEBUG_PIXEL
    ,DISABLE_PREVIEW
    ,SAMPLES_PER_LIGHT
    ,SAMPLES_PER_PIXEL
};

/******************************************************************************/

const option::Descriptor usage[] =
{
    {
         UNKNOWN
        ,0
        ,"" 
        ,""
        ,option::Arg::None_
        ,"USAGE: <program> [options]\n\n Options:" 
    },
    {
         HELP
        ,0
        ,""
        ,"help"
        ,option::Arg::None_
        ,"  --help  \t\tPrint usage and exit."
    },
    {
         RUN_TESTS_AND_QUIT
        ,0
        ,"T"
        ,"tests"
        ,option::Arg::None_
        ,"  --tests \t\tRun tests and exit."
    },
    {
         PRINT_CONFIG_AND_QUIT
        ,0
        ,"C"
        ,"config"
        ,option::Arg::None_
        ,"  --config \t\tPrint the configuration to stdout and quit."
    },
    {
         PRINT_CAMERA
        ,0
        ,""
        ,"camera"
        ,option::Arg::None_
        ,"  --camera \t\tPrint the camera to stdout."
    },
    {
         DEBUG_PIXEL
        ,0
        ,"D"
        ,"debug"
        ,option::Arg::Optional
        ,"  --debugX --debugY or -D240 -D 130 \t\tDebugs the output for pixel at coordinate <x,y>."
    },
    {
         DISABLE_PREVIEW
        ,0
        ,"P"
        ,"no-preview"
        ,option::Arg::None_
        ,"  -P/--no-preview \t\tDisable preview mode and begin rendering immediately."
    },
    {
         SAMPLES_PER_LIGHT
        ,0
        ,"S"
        ,"light-samples"
        ,option::Arg::Optional
        ,"  -S/--light-samples \t\tSpecifies the number of shadow rays to be sampled."
    },
    {
         SAMPLES_PER_PIXEL
        ,0
        ,"A"
        ,"aa"
        ,option::Arg::Optional
        ,"  -A/--aa \t\tSpecifies the number of primary rays used to sample each pixel."
    },
    {
         UNKNOWN
        ,0
        ,""
        ,""
        ,option::Arg::None_
        ,"\nExamples:\n"
         "  example --unknown -- --this_is_no_option\n"
         "  example -unk --plus -ppp file1 file2\n" 
    }
};

/******************************************************************************/

#endif
