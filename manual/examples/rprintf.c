#include <stdio.h>
#include <printf.h>
#include <stdarg.h>

/*@group*/
typedef struct
  {
    char *name;
  } Widget;
/*@end group*/

int 
print_widget (FILE *stream, const struct printf_info *info, va_list *app)
{
  Widget *w;
  char *buffer;
  int len;

  /* Format the output into a string. */
  w = va_arg (*app, Widget *);
  len = asprintf (&buffer, "<Widget %p: %s>", w, w->name);
  if (len == -1)
    return -1;

  /* Pad to the minimum field width and print to the stream. */
  len = fprintf (stream, "%*s",
		 (info->left ? - info->width : info->width),
		 buffer);

  /* Clean up and return. */
  free (buffer);
  return len;
}


int
main (void)
{
  /* Make a widget to print. */
  Widget mywidget;
  mywidget.name = "mywidget";

  /* Register the print function for widgets. */
  register_printf_function ('W', print_widget, NULL); /* No arginfo.  */

  /* Now print the widget. */
  printf ("|%W|\n", &mywidget);
  printf ("|%35W|\n", &mywidget);
  printf ("|%-35W|\n", &mywidget);

  return 0;
}
