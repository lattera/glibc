struct link_map_machine
  {
    Elf32_Addr plt; /* Address of .plt + 0x2c */
    Elf32_Addr gotplt; /* Address of .got + 0x0c */
  };
