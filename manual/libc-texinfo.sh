#! /bin/sh

# Create libc.texinfo from the chapter files.

grep '^@node.*Top' $1 | cut -d, -f-2 |
    sed 's/, /:/; s/:@node /:/; s/ /_/g; s/:/ /g' >cnodes.$$

$AWK '{ file[$2] = $1; nnode[$2] = $3 }
END  { for(x in file)
	if(file[x] != "")
	    print file[x] ":" x, file[nnode[x]] ":" nnode[x] }' \
    cnodes.$$ | tsort | sed 's/_/ /g; $d' >corder.$$

[ -z "$2" ] || grep '^@node.*Top' `echo $2 /dev/null | tr ' ' '\n' | sort` |
    cut -d, -f1 | sed 's/@node //' >xorder.$$

grep '^@node.*Top' $3 | cut -d, -f-2 |
    sed 's/, /:/; s/:@node /:/; s/ /_/g; s/:/ /g' >anodes.$$

$AWK '{ file[$2] = $1; nnode[$2] = $3 }
END  { for(x in file)
	if(file[x] != "")
	    print file[x] ":" x, file[nnode[x]] ":" nnode[x] }' \
    anodes.$$ | tsort | sed 's/_/ /g; $d' >aorder.$$

IFS=:

>incl.$$
>smenu.$$
>lmenu.$$

while read file node; do
    echo "@include $file" >>incl.$$
    echo "* $node:: `sed -n 's/^@c %MENU% //p' $file`" >>smenu.$$
    lmenu=`sed -n '/^@menu/,/^@end menu/p; /^@end menu/q' $file |
	sed '/^@menu/d; /^@end menu/d'`
    [ -z "$lmenu" ] || (
	echo; echo "$node"; echo
	echo "$lmenu"
    ) >>lmenu.$$
done <corder.$$

if [ -f xorder.$$ ]; then

    (echo; echo 'Add-ons'; echo) >>smenu.$$

    while read file node; do
	echo "@include $file" >>incl.$$
	echo "* $node:: `sed -n 's/^@c %MENU% //p' $file`" >>smenu.$$
	lmenu=`sed -n '/^@menu/,/^@end menu/p; /^@end menu/q' $file |
	    sed '/^@menu/d; /^@end menu/d'`
	[ -z "$lmenu" ] || (
	    echo; echo "$node"; echo
	    echo "$lmenu"
	) >>lmenu.$$
    done <xorder.$$
fi

(echo; echo 'Appendices'; echo) >>smenu.$$

while read file node; do
    echo "@include $file" >>incl.$$
    echo "* $node:: `sed -n 's/^@c %MENU% //p' $file`" >>smenu.$$
    lmenu=`sed -n '/^@menu/,/^@end menu/p; /^@end menu/q' $file |
	sed '/^@menu/d; /^@end menu/d'`
    [ -z "$lmenu" ] || (
	echo; echo "$node"; echo
	echo "$lmenu"
    ) >>lmenu.$$
done <aorder.$$

$AWK '
BEGIN { FS=":" }

/^\*/ {
  printf("%-32s", $1 "::");
  x = split($3, word, " ");
  hpos = 34;
  for(i = 1; i <= x; i++) {
    hpos += length(word[i]) + 1;
    if(hpos > 78) {
      printf("\n%34s", "");
      hpos = 35 + length(word[i]);
    }
    printf(" %s", word[i]);
  }
  print ".";
}

!/^\*/ { print; }
' smenu.$$ >smenux.$$

mv -f incl.$$ chapters.texi

(echo '@menu'
 cat smenux.$$
 cat <<EOF
* Copying::                      The GNU Library General Public License says
                                  how you can copy and share the GNU C Library.

Indices

* Concept Index::                Index of concepts and names.
* Type Index::                   Index of types and type qualifiers.
* Function Index::               Index of functions and function-like macros.
* Variable Index::               Index of variables and variable-like macros.
* File Index::                   Index of programs and files.

 --- The Detailed Node Listing ---
EOF
 cat lmenu.$$
 echo '@end menu' ) >top-menu.texi.$$
mv -f top-menu.texi.$$ top-menu.texi

rm -f *.$$
