#!/bin/sh

echo 'memory_initialization_radix=16;'
echo 'memory_initialization_vector='
xxd -ps $1 | tr -d '\n' | sed -e 's#\([^,]\{8\}\)#\1,#g' -e 's#\(\([^,]\{2\}\)\([^,]\{2\}\)\([^,]\{2\}\)\([^,]\{2\}\)\)#\5\4\3\2#g' -e 's#,$#;#' -e 's#,#-,#g' | tr , '\n' | sed -e 's#-#,#g'
