$first=$last=$idx=0;
while (<>) {
  local($ucs,$rest) = split;
  local($u)=hex($ucs);
  if ($u - $last > 6) {
    if ($last != 0) {
      printf ("  { start: 0x%04x, end: 0x%04x, idx: %5d },\n",
	      $first, $last, $idx);
      $idx -= $u - $last - 1;
    }
    $first=$u;
  }
  $last=$u;
}
printf ("  { start: 0x%04x, end: 0x%04x, idx: %5d },\n",
	$first, $last, $idx);
