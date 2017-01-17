/* HTTP Parser for MPID */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

// Моя реализация strncpy. Нагло сворована с man-страницы этой функции =)
// В отличие от стандартной, заканчивает сроку нулевым символом при любом раскладе
char* strncpy_p(char *dest, const char *src, size_t n)
{
  size_t i;

  for (i = 0 ; i < n && src[i] != '\0' ; i++)
    dest[i] = src[i];
    
  for ( ; i < n ; i++)
    dest[i] = '\0';

  if (n > 0)
    dest[n - 1] = '\0';
  
  return dest;
}

int parse_input(char *data, http_info *i)
{
  char *tmp;
  int pos;
  
  if(((long)data == 0)||((long)i == 0)) return -1;
  // Определяем метод
  switch(data[0])
  {
    case 'G':
      i->method = METHOD_GET;
      break;
      
    case 'P':
      switch(data[1])
      {
	case 'O':
	  i->method = METHOD_POST;
	  break;
	case 'U':
	  i->method = METHOD_PUT;
	  break;
	default:
	  return -2;
	  break;
      }
      break;
      
    default:
      return -2;
      break;
  };
  
  // Вычленяем имя файла, к которому производилось обращение
  while((data[0] != '/') && (data[0] != '\0')) *data++;
  if(data[0] == 0) return -3;
  
  tmp = data;
  pos = 0;
  
  while((data[0] != ' ') && (data[0] != '?') && (data[0] != '\0')) { *data++; pos++; }
  if(data[0] == 0) return -3;
  
  pos++;	//strncpy_p HACK
  
  strncpy_p(i->fname, tmp, (pos >= FNAME_LIMIT ? FNAME_LIMIT : pos));
  
  //printf("%s\n", i->fname);
  
  // Выберем аргументы, если они есть
  if(data[0] == ' ')
  {
    i->args[0] = '\0';
  } else {
    *data++;
    if(data[0] == 0) return -4;
    tmp = data;
    pos = 0;
    
    while((data[0] != ' ') && (data[0] != '\0')) { *data++; pos++; }
    if(data[0] == 0) return -5;
    
    pos++;	//strncpy_p HACK
  
    strncpy_p(i->args, tmp, (pos >= ARGS_LIMIT ? ARGS_LIMIT : pos));
  }
  
  //printf("%s\n", i->args);
  
  // Найдем и определим версию протокола
  *data++;
  if(data[0] == 0) return -6;
  
  while((data[0] != '.') && (data[0] != '\0')) { *data++;}
  if(data[0] == 0) return -7;
  
  *data++;
  if(data[0] == 0) return -8;
  
  if(data[0] == '0') i->http_ver = HTTP_10;
  else if(data[0] == '1') i->http_ver = HTTP_11;
  else return -9;
  
  while((data[0] != '\n') && (data[0] != '\0')) { *data++;}
  if(data[0] == 0) return -10;
  
  *data++;
  if(data[0] == 0) return 0; //-11; //HACK
  
  // Выведем остаток
  //printf("%s\n", data);
  
  // Теперь понеслись по кочкам - разбираем оставшиеся заголовки
  
  while(1)
  {
    //printf("________________________\n");
    char *name, *value;
    int p1, p2;
    
    if((data[0] == '\r')||(data[0] == '\n'))	// Нашли пустой заголовок - обработка закончена
    {
      *data++;
      if(data[0] == '\n')
      {
	*data++;
      }
      
      break;
    }
      
    name = data;
    p1 = 0;
    
    while((data[0] != ':') && (data[0] != '\0')) { *data++; p1++; }
    if(data[0] == 0) return -12;
    
    while(((data[0] == ':') || (data[0] == ' ')) && (data[0] != '\0')) { *data++; }
    if(data[0] == 0) return -13;
    
    value = data;
    p2 = 0;
    
    //printf("data = %s|EndData\n", data);
    
    while((data[0] != '\r')&&(data[0] != '\n') && (data[0] != '\0')) { *data++; p2++; }
    if(data[0] == 0) return -14;
    if(data[0] == '\r') *data++;
    p2++;	//strncpy_p HACK
    
    *data++;
    if((data[0] == 0)) 
    {
      if(i->method == METHOD_GET) return -15;	//Грязный, грязный HACK
      else return 0;
    }
    
    // Собрали заголовок и его значение. Теперь сравним заголовок с известными
    // и в случае совпадения запишем значение в структуру
#define NAME_EQUAL(x) (strncasecmp(name, x, p1) == 0)   

    if(NAME_EQUAL("Host")) 
    {
      strncpy_p(i->host, value, (p2 >= HOST_LIMIT ? HOST_LIMIT : p2));
      continue;
    }
    
    if(NAME_EQUAL("User-Agent"))
    {
      strncpy_p(i->uagent, value, (p2 >= UAGENT_LIMIT ? UAGENT_LIMIT : p2));
      continue;
    }
    
    if(NAME_EQUAL("Referer"))
    {
      strncpy_p(i->referer, value, (p2 >= REFERER_LIMIT ? REFERER_LIMIT : p2));
      continue;
    }
    
    if(NAME_EQUAL("Accept"))
    {
      strncpy_p(i->accept, value, (p2 >= ACCEPT_LIMIT ? ACCEPT_LIMIT : p2));
      continue;
    }
    
    if(NAME_EQUAL("Content-Length"))
    {
      strncpy_p(i->contlen, value, (p2 >= CONT_LIMIT ? CONT_LIMIT : p2));
      continue;
    }
    
    if(NAME_EQUAL("Content-type"))
    {
      
      strncpy_p(i->conttype, value, (p2 >= CONTT_LIMIT ? CONTT_LIMIT : p2));
      //i->conttype[p2 - 2] = 0;
      //printf("\\%s/\n", i->conttype);
      continue;
    }
    
    if(NAME_EQUAL("Cookie"))
    {
      strncpy_p(i->cookie, value, (p2 >= COOKIE_LIMIT ? COOKIE_LIMIT : p2));
      continue;
    }

    //printf("Unknown header!\n");
  }
  
  // Если данные пришли методом POST, то после всех заголовков вполне могут найтись
  // отправленные браузером данные
  if(i->method == METHOD_POST)
  {
    i->postdata = data;
    //printf("POST len %s, data: %s\n", i-> contlen, i->postdata);
  }
  
  return 0;
  
}
#ifdef __CONFTEST__
char *test = "POST /wiki/install.php?l=ru HTTP/1.1\r\n\
Host: localhost:8080\r\n\
Connection: Keep-Alive\r\n\
User-Agent: Mozilla/5.0 (compatible; Konqueror/4.3; Linux) KHTML/4.3.5 (like Gecko)\r\n\
Referer: http://localhost:8080/wiki/install.php?l=ru\r\n\
Pragma: no-cache\r\n\
Cache-control: no-cache\r\n\
Accept: text/html, image/jpeg;q=0.9, image/png;q=0.9, text/*;q=0.9, image/*;q=0.9, */*;q=0.8\r\n\
Accept-Encoding: x-gzip, x-deflate, gzip, deflate\r\n\
Accept-Charset: utf-8, utf-8;q=0.5, *;q=0.5\r\n\
Accept-Language: ru, en-GB, en-US, en\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\r\n";

int main()
{
  http_info *i  = malloc(sizeof(http_info));
  int res;
  
  res = parse_stream(NULL, i);
  printf("res = %d\n", res);
  
  res = parse_stream(test, NULL);
  printf("res = %d\n", res);  
  
  res = parse_stream(test, i);
  printf("res = %d\n", res);
  
  if(res == 0)
  {
    printf("Method %d\n", i->method);
    printf("Requested file %s\n", i->fname);
    printf("Arguments: %s\n", i->args);
    printf("HTTP version: %d\n", i->http_ver);
    printf("User agent: %s\n", i->uagent);
    printf("Referer: %s\n", i->referer);
    printf("Host: %s\n", i->host);
    printf("Accept: %s\n", i->accept);
    printf("Done.\n");
  }
  
  free(i);
}
#endif
