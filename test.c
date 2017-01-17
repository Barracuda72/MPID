#include "libmime.h"

int main()
{
  
  printf("%s\n", get_content_type("HtMl"));
  printf("%s\n", get_content_type("xmL"));
  printf("%s\n", get_content_type("txt"));
  printf("%s\n", get_content_type("JPPG"));
  printf("%s\n", get_content_type("Jpeg"));
  printf("%s\n", get_content_type("FLv"));
  printf("%s\n", get_content_type("Gif"));
  printf("%s\n", get_content_type("PnG"));
  printf("%s\n", get_content_type("gZ"));
  printf("%s\n", get_content_type("bz2"));

  load_mime_db();
  
  printf("%s\n", get_content_type("HtMl"));
  printf("%s\n", get_content_type("xmL"));
  printf("%s\n", get_content_type("txt"));
  printf("%s\n", get_content_type("JPPG"));
  printf("%s\n", get_content_type("Jpeg"));
  printf("%s\n", get_content_type("FLv"));
  printf("%s\n", get_content_type("Gif"));
  printf("%s\n", get_content_type("PnG"));
  printf("%s\n", get_content_type("gZ"));
  printf("%s\n", get_content_type("bz2"));
}
