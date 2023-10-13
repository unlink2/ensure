#!/bin/bash

# rename to project specific values
NAME="ensr"
CONST="ENSR"
STRUCT="ensr"
FN="ensr"

# will be replaced
ensr_NAME="ensr"
ensr_STRUCT="ensr"
ensr_CONST="ensr"
ensr_FN="ensr"

function replace() {
	echo "Replacing $1 with $2"
	find ./ -type f -name '*.c' -exec sed -i "s/$1/$2/g" {} \;
	find ./ -type f -name '*.h' -exec sed -i "s/$1/$2/g" {} \;
	find ./ -type f -name '*.md' -exec sed -i "s/$1/$2/g" {} \;
	find ./ -type f -name '*.lua' -exec sed -i "s/$1/$2/g" {} \;
	find ./ -type f -name '*.sh' -exec sed -i "s/$1/$2/g" {} \;
	find ./doc/ -type f -name '*' -exec sed -i "s/$1/$2/g" {} \;
}

replace $ensr_NAME $NAME
replace $ensr_CONST $CONST
replace $ensr_STRUCT $STRUCT
replace $ensr_FN $FN

mv "include/$ensr_NAME.h" "include/$NAME.h"
mv "src/$ensr_NAME.c" "src/$NAME.c"
mv "doc/$ensr_NAME" "doc/$NAME" 
