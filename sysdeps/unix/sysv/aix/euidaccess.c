int
euidaccess (const char *name, int type)
{
  return accessx (name, type, ACC_SELF);
}
