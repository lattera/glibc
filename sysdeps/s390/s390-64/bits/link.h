struct link_map_machine
  {
    Elf64_Addr plt; /* Address of .plt + 0x2e */
    Elf64_Addr gotplt; /* Address of .got + 0x18 */
  };
