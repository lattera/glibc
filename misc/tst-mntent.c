/* Test case by Horst von Brand <vonbrand@sleipnir.valparaiso.cl>.  */
#include <stdio.h> 
#include <mntent.h> 
 
int
main (int argc, char *argv[]) 
{ 
  int result = 0;
  struct mntent mef =
  { 
     "/dev/hda1", "/", "ext2", "defaults", 1, 1 
  }; 
  struct mntent *mnt = &mef; 
 
  if (hasmntopt (mnt, "defaults"))  
    printf("Found!\n"); 
  else 
    {
      printf("Didn't find it\n"); 
      result = 1;
    }
   
   return result; 
} 
