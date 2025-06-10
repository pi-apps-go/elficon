#!/bin/sh

find_app()
{
	FILENAME="$1";
	if [ -e "${FILENAME}" ]; then
		FOUNDFILE="${FILENAME}";
	else
		FOUNDFILE=`which ${FILENAME}`;
	fi
	echo "${FOUNDFILE}";
}

find_icon()
{
	FOLDERLIST=" \
		/usr/share/icons/gnome/scalable/* \
		/usr/share/icons/gnome/48x48/apps \
		/usr/share/icons/Humanity/apps/48 \
		/usr/share/icons/Human/48x48/apps \
		/usr/share/icons/oxygen/48x48/apps \
		/usr/share/icons/hicolor/48x48/apps \
		/usr/share/icons/gnome/*/apps \
		/usr/share/icons/Humanity/apps/* \
		/usr/share/icons/Human/*/apps \
		/usr/share/icons/oxygen/*/apps \
		/usr/share/icons/hicolor/*/apps \
		/usr/share/icons/gnome/*/* \
		/usr/share/icons/Humanity/*/* \
		/usr/share/icons/Human/*/* \
		/usr/share/icons/oxygen/*/* \
		/usr/share/icons/hicolor/*/* \
		/usr/share/pixmaps \
	";
	FILENAME="$1";
	if [ -e "${FILENAME}" ]; then
		FOUNDFILE="${FILENAME}";
	else
		FOUNDFILE="";
		for FOLDER in ${FOLDERLIST}; do
			FILE=`ls ${FOLDER}/${FILENAME}*` 2> /dev/null;
			if [ -e "${FILE}" ]; then
				FOUNDFILE="${FILE}";
				break;
			fi
		done
	fi
	echo "${FOUNDFILE}";
}

echo "Warning: This will replace many system binaries, it is unlikely (though always possible) that some of these binaries do a hash check on themselves.  Any application that does this would be rendered unusable.";
echo -n "Do you wish to continue? [y/N]"
read CONTINUE
if [ "${CONTINUE}" != "y" ]; then
	exit;
fi

APPS="/usr/share/applications";
for FILE in `ls "${APPS}"`; do
	EXEC=`cat "${APPS}/${FILE}" | awk 'BEGIN {FS="[= ]";} $1=="Exec" {print $2;}'` 2> /dev/null;
	ICON=`cat "${APPS}/${FILE}" | awk 'BEGIN {FS="=";} $1=="Icon" {print $2;}'` 2> /dev/null;
	if [ -n "${ICON}" ]; then
		EXECFILE=`find_app $EXEC`;
		ICONFILE=`find_icon $ICON`;
		if [ -n "${ICONFILE}" ]; then
			cp $EXECFILE $EXECFILE.bak;
			elficon -s $EXECFILE `uuidgen`;
			elficon -a $EXECFILE "${ICON}" "${ICONFILE}";
		fi
	fi
done

