#!/bin/sh

LABBR="de";
LNAME="German";

dpkg -l "language-pack-${LABBR}" 2>&1 > /dev/null;
RET=$?;
if [ "${RET}" -ne "0" ]; then
	echo "The ${LNAME} language pack (language-pack-${LABBR}) is required to use this test.";
	exit;
fi
export LC_ALL="de_DE.UTF-8";
echo "The prompt is now in the ${LNAME} language, use \"exit\" to return to the default language.";
/bin/bash;
export LC_ALL="";
