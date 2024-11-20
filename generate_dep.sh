#!/usr/bin/bash
# Usage: generate_dep.sh BASE_COMMAND_LINE DEPENDENCY_NAME
# Creates a dependency file DEPENDENCY_NAME.d using the CPP command and
# configuration given in BASE_COMMAND_LINE.

raw="`$1 -MM $2.cpp -MT '' -MG`"
if [ $? -ne 0 ]
then
  echo Dependency generation failed. >&2
  exit 1
fi
# remove leading colon and space
dep_list=${raw:2}
(
  echo "$2.o: $dep_list"
  # don't depend on files that don't exist anymore (and won't exist as generated files)
  echo "$2.d: \$(filter \$(hdr) \$(resh),$dep_list)"
  echo -e '\t./generate_dep.sh "$(CXX)"' "$2"
) > "$2.d"
