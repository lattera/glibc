#if __WORDSIZE == 64
struct link_map_machine
  {
    Elf64_Addr plt; /* Address of .plt + 0x2e */
    Elf64_Addr gotplt; /* Address of .got + 0x18 */
  };
#else
struct link_map_machine
  {
    Elf32_Addr plt; /* Address of .plt + 0x2c */
    Elf32_Addr gotplt; /* Address of .got + 0x0c */
  };
#endif
