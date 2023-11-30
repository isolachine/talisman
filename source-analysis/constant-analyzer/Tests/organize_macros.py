#!/usr/bin/pytho

import sys
import re

if len(sys.argv) != 2:
    print "Usage: " + sys.argv[0] + " <taint_constants> <clang_constants>"

pp_trace_output = sys.argv[1]

trace_file = open(pp_trace_output)


current_macro_label = ""

for line in trace_file:
	if "MacroNameTok" in line:
		line_no_newline = line.split("\n")[0]
		current_macro_label = line_no_newline.split(' ')[3]

	if "Range" in line:
		if ("ConditionRange" not in line) and ("FilenameRange" not in line) and ("SourceRangeSkipped" not in line) and ("nonfile" not in line):
			if (current_macro_label != ""):
				line_no_newline = line.split("\n")[0]
				range_substring = line_no_newline.split(' ')[3]
				file_1 = range_substring.split('[')[1]
				file_substring = file_1.split(',')[0]
				file_array = file_substring.split('"')[1].split(':')
				trimmed_location = file_array[0] + ':' + file_array[1]
				print current_macro_label + ':' + trimmed_location

