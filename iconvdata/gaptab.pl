$first=$last=$idx=0;
sub fmt {
  printf ("\n ") if (($n % 8) == 0);
  ++$n;
  my($val) = pop(@_);
  printf (" '\\x%02x',", $val);
}
while (<>) {
  local($ucs,$char,%rest) = split;
  local($u)=hex($ucs);
  local($c)=hex($char);
  if ($u - $last > 6) {
    if ($last != 0) {
      $idx += $last - $first + 1;
    }
    $first=$u;
  } else {
    for ($m = $last + 1; $m < $u; ++$m) {
      fmt (0);
    }
  }
  fmt ($c);
  $last=$u;
}
printf ("\n");
