struct link_map_machine
  {
    Elf32_Addr plt; /* Address of .plt + 0x16 */
    Elf32_Addr gotplt; /* Address of .got + 0x0c */
  };
