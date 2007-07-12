#!/bin/awk -f
/^cvs (diff|server): tag.*is not in file/ {
	print
	next
}
/^cvs (diff|server): Diffing/ {
	next
}
/^\? / {
	next
}
/^Index:/ {
	file=$2
	next
}
/^===============/ {
	next
}
/^RCS/ {
	next
}
/^retrieving/ {
	next
}
/^diff/ {
	next
}
/^---/ {
	if ($2 ~ /^\/dev\/null/ ||
	    $2 ~ /^\/tmp\//)
		$2=file
	if ($2 ~ /.cvsignore$/ ||
	    $2 ~ /^c_stubs/ ||
	    $2 ~ /^rtkaio/ ||
	    $2 ~ /^powerpc-cpu/ ||
	    $2 ~ /^fedora/ ||
	    $2 ~ /^localedata\/charmaps\/GB18030/ ||
	    $2 ~ /^iconvdata\/gb18030\.c/) {
		hide = 1
		next
	} else {
		hide = 0
	}
	sub(/^---[ 	]*/,"--- " OLDVER "/")
}
/^\+\+\+/ {
	if (hide)
		next
	if ($2 ~ /^\/dev\/null/ ||
	    $2 ~ /^\/tmp\//)
		$2=file
	sub(/^\+\+\+[ 	]*/,"+++ " NEWVER "/")
}
{
	if (hide)
		next
	print
}
