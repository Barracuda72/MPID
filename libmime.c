/* Mime types library */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libmime.h>

typedef struct _mimetype
{
  char	ext[8];
  char	type[256];
} mimetype;
/*
mimetype typesdb[] = {
  //{"", ""},
  {"123",  "application/vnd.lotus-1-2-3"},
  {"3ds",  "image/x-3ds"},
  {"669",  "audio/x-mod"},
  {"a",    "application/x-archive"},
  {"abw",  "application/x-abiword"},
  {"ac3",  "audio/ac3"},
  {"adb",  "text/x-adasrc"},
  {"ads",  "text/x-adasrc"},
  {"afm",  "application/x-font-afm"},
  {"ag",   "image/x-applix-graphics"},
  {"ai",   "application/illustrator"},
  {"aif",  "audio/x-aiff"},
  {"aifc", "audio/x-aiff"},
  {"aiff", "audio/x-aiff"},
  {"",""}
};*/

mimetype typesdb[1024];

int typesdbsz = 0;//sizeof(typesdb)/sizeof(mimetype);

int _init()
{
  load_mime_db();
}

int _fini()
{
}

int load_mime_db()
{
#define STRLEN 1024
  char *buf = malloc(STRLEN);
  char *ext;
  char *type;
  
  typesdbsz = 0;
  
  FILE *f = fopen("mimedb.txt", "r");
  
  if(f != NULL)
  {
    while(fgets(buf, STRLEN, f) != NULL)
    {
      if(buf[strlen(buf) - 1] == '\n')
      {
	buf[strlen(buf) - 1] = '\0';
      }
      
      if(buf[0] == '#') continue;
      if(strchr(buf, '=') == 0) continue;
      
      type = strtok(buf, "=");
      
      while ((ext = strtok(NULL, " ")) != NULL)
      {
	strcpy(typesdb[typesdbsz].ext, ext);
	strcpy(typesdb[typesdbsz].type, type);
	typesdbsz++;
      }
    }
    
    fclose(f);
  }
  
  /*int i = 0;
  for(;i < typesdbsz; i++) printf("%s: %s\n", typesdb[i].ext, typesdb[i].type);*/
}

char *get_content_type(char *ext)
{
  //printf("%d\n", typesdbsz);
  int i;
  for (i = 0; i < typesdbsz; i++)
  {
    if(strcasecmp(typesdb[i].ext, ext) == 0) return typesdb[i].type;
  }
  return "text/plain";
}