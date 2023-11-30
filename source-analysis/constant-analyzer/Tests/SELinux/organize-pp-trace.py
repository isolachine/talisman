#!/usr/bin/python

import sys
import re

DEBUG = 0

if len(sys.argv) != 3:
    print "Usage: " + sys.argv[0] + " <local-define-file> <pp-trace-output-file>"

local_macro = sys.argv[1]
pp_trace = sys.argv[2]


### Parsing local macros is achieved in the following manner
### - Change directory to a particular LSM or directory with source code
### - Perform the following command: grep -R "#define" &> ~/output_filename.txt
### - Feed output as first arg to this script
###

local_macro_file = open(local_macro)

local_macro_dict = {}
new_filler = 0

for line in local_macro_file:
    if ("/*" not in line) and ("*/" not in line):
        if ("_H" not in line):
            line = line.split('\n')[0]
            #local_macro_name = line.split("#define ")[1].split('\n')[0]
            local_macro_name = line.split("#define ")
            if (len(local_macro_name) < 2):
                local_macro_name = line.split("\t")[1]
            else:
                local_macro_name = line.split("#define ")[1]

            local_macro_name_no_ws = ""
            if (len(local_macro_name.split(' ')) > 1):
                local_macro_name_no_ws = local_macro_name.split(' ')[0].split('(')[0]
            elif (len(local_macro_name.split('\t')) > 1):
                local_macro_name_no_ws = local_macro_name.split('\t')[0].split('(')[0]
            else:
                local_macro_name_no_ws = local_macro_name.split(' ')[0]

            #print local_macro_name_no_ws


            #local_macro_name_no_value = local_macro_name.split(' ')
            #local_macro_name_no_call = local_macro_name.split('(')[0]

            #print local_macro_name_no_call

            if (local_macro_dict.get(local_macro_name_no_ws) != 1):
                local_macro_dict[local_macro_name_no_ws] = 1

#if (DEBUG):
#for key in local_macro_dict.keys():
    #print key


### The format for pp-trace is as follows:
### The line MacroNameTok will contain the macro name
###     - Each subsequent line will contain metadata regarding the macro
###       until another MacroNameTok line specified a new macro.
###     - Metadata lines containing "Range" or "Loc" contain location information.
###       I've found range to be more appropriate in general cases

pp_trace_file = open(pp_trace)

current_macro = ""

macro_dict = {}

filler = 0;

for line in pp_trace_file:
    #This line contains the name of the
    if ("MacroNameTok" in line):
        line = line.split('\n')[0]
        # These are spurious filler data when pp-trace is unable to find
        # certain metadata. Ignore these lines.
        if ("notrace" not in line) and ("inline" not in line) and ("NULL" not in line):
            line_array = line.split(": ")
    	    current_macro = line_array[1]

            # Here we check that the current macro is not locally defined
            if (current_macro not in local_macro_dict.keys()):
                # Check if macro has already been visited
                if (current_macro not in macro_dict.keys()):
                    new_macro_meta_list = []
                    macro_dict[current_macro] = new_macro_meta_list
            # Otherwise reset macro
            else:
                current_macro = ""
        # Otherwise reset macro
        else:
            current_macro = ""

    if ("Range" in line):
        line = line.split('\n')[0]
        if ("ConditionRange" not in line) and ("FilenameRange" not in line) and ("SourceRangeSkipped" not in line) and ("nonfile" not in line) and ("error generated" not in line):
            if (current_macro in macro_dict.keys()):

                first_line_number = line.split(':')[2]
                first_column_number = line.split(':')[3].split('"')[0]

                second_line_number = line.split(':')[4]
                second_column_number = line.split(':')[5].split('"')[0]

                if (int(first_line_number) == int(second_line_number)) and (int(first_column_number) == int(second_column_number)):
                    first_range = line.split(' ')[3].split('[')[1].split(',')[0].split('"')[1]
                    macro_dict[current_macro].append(first_range)


if (DEBUG):
    for key in macro_dict.keys():
        if len(macro_dict[key]) != 0:
            print key + " : "
            print macro_dict[key]


output_file = pp_trace.split('.')[0] + "_organized_pp_trace.txt"
f_out = open(output_file, "w")

#if (DEBUG):
for key in macro_dict.keys():
    if len(macro_dict[key]) != 0:
        macro_plus_key = "macro:" + key + "\n"
        f_out.write(macro_plus_key)
        for range in macro_dict[key]:
            range_plus_key = "\trange:" + range + "\n"
            f_out.write(range_plus_key)
