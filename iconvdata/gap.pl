$first=$last=$idx=0;
while (<>) {
  local($ucs,%rest) = split;
  local($u)=hex($ucs);
  if ($u - $last > 6) {
    if ($last != 0) {
      printf ("  { start: %#06x, end: %#06x, idx: %5d },\n",
	      $first, $last, $idx - $first);
      $idx += $last - $first + 1;
    }
    $first=$u;
  }
  $last=$u;
}
printf ("  { start: %#06x, end: %#06x, idx: %5d },\n",
	$first, $last, $idx - $first);
