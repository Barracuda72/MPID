/* HTTP protocol module for MPID */

/*System includes */
//#include <errno.h>
//#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE	
//for strchrnul, basename
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
//#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
//#include <dlfcn.h>

/* Own includes */
#include <libproto.h>
#include <libmime.h>
#include <http_ans.h>
#include <parser.h>
#include <htconfig.h>

//extern char **environ;

/* My realization of strncpy, because I need null-terminated string */


char* strncpy_m(char *dest, const char *src, size_t n)
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

/* Global variables */
//char htmldir[] = "/var/www/";
//char index_name[] = "/index.html";
//char php_location[] = "/usr/bin/php-cgi";

/*char *codes[] = 
{
  "HTTP/1.1 200 OK\r\n",
  "HTTP/1.1 400 Bad Request\r\n",
  "HTTP/1.1 404 Not Found\r\n",
  "HTTP/1.1 403 Forbidden\r\n"
};*/

char *head = "HTTP/1.1 %d %s\r\n";

char *errpage = "\
<html><head><title>%s</title></head>\
<body><h1>There was an error here:</h1>\
<br/>%s<br/><hr/><br/>\
<i>MPID - free web server, created by Barracuda (c)2010-2017</i></body></html>\
";

char *errhead = "Connection: close\r\nContent-type: text/html\r\n\r\n";

htconf cfg =
{
  "defname",
  "/tmp",
  "index.html",
  0,
  0,
  {0},
  {0}
};

int init_module(void)
{
  //load_mime_db();
  //cfg = malloc(sizeof(cfg));
  int i;
  if((i = read_config("htconfig.cfg", &cfg)) != 0) printf("Config error %d\n", i);
  return EXIT_SUCCESS;
}

int fini_module(void)
{
  //free(cfg);
}

int send_answer(int sockFd, int num)
{
    char data[256];
    sprintf(data, head, num, "Unimplemented");
    send(sockFd, data, strlen(data), (long)NULL);
    if((num != HTTP_200)&&(num != HTTP_302))
    {
      send(sockFd, errhead, strlen(errhead), 0);
      char *ans = malloc(16384);
      sprintf(ans, errpage, data, data);
      send(sockFd, ans, strlen(ans), (long)NULL);
      free(ans);
    }
}

int find_index_page(char *vars[], int indcnt, char *dir, char *indname)
{
  int pos;
  char *tmp;
  char path[1024];
  int i; 
 
  for (i = 0; i < indcnt; i++)
  {
    strncpy_m(indname, vars[i], 1024);
    //printf("Check index file %s...\n", indname);
    strncpy_m(path, dir, 1024);
    strcat(path, indname);
    FILE *f;
    if((f = fopen(path, "r")) != 0)
    {
      fclose(f);
      break;
    }
  }
  
  return 0;
  //strcpy(indname, "index.html");
}

char *getCGIHandler(char *ext)
{
  int i;
  for(i = 0; i < cfg.extcnt; i++)
  {
    if(strncasecmp(cfg.ext[i], ext, 16) == 0) return cfg.exe[i];
  }
  return NULL;
}

int handle_connection(int sockId, struct sockaddr_in *cli)
{
  printf("\n----------------------------------------\n");
  char clientIp[16];
  char port[8];
  inet_ntop(AF_INET, &(cli->sin_addr), clientIp, sizeof(clientIp));	// Получили IP
  sprintf(port, "%d", ntohs(cli->sin_port));
  printf("Connection from [%s:%d]\n", clientIp, ntohs(cli->sin_port));
  
  int status = 0;
  int length = 0;
  char *answer = malloc(16384);
  
  char *tmpbuf = NULL;
  int tmpsize = 0;

  length = 64*1024;
  do {
    tmpbuf = realloc(tmpbuf, tmpsize+64*1024);
    memset(tmpbuf+tmpsize, '\0', 64*1024);
    length = recv(sockId, tmpbuf+tmpsize, 64*1024, (long)NULL/*MSG_WAITALL*/);
    tmpsize += length;
  } while (length == 64*1024);
  
  printf("Reseived:\n");
  int i = 0;
  for(i=0;i<tmpsize;i++) putchar(tmpbuf[i]);
  printf("\nEnd\n");
  
  http_info *in = malloc(sizeof(http_info));
  if((i = parse_input(tmpbuf, in)) != 0)
  {
    printf("Parser error %d\n", i);
    send_answer(sockId, HTTP_400);
    return CONN_NEED_CLOSE;
  }
  
  char *path = malloc(16384);
  //fprintf(stdout, "path = %s, args = %s\n", in->fname, in->args);
  sprintf(path, "%s%s", cfg.sroot, in->fname);
  
  DIR *a;
  
  if((a = opendir(path)) != NULL)
  {
    closedir(a);
    if(path[strlen(path) - 1] != '/') strcat(path, "/");
    char *indexn = malloc(1024);
    if(find_index_page(cfg.index, cfg.indcnt, path, indexn) != 0)
    {
      if(cfg.dirind == 0) 
      {
	send_answer(sockId, HTTP_403);
	status = 1;
      }
    }
    strcat(path, indexn);
    free(indexn);
  }

  //char *path = malloc(16384);
  
  //strcpy(path, tmpb2);
  
  //free(tmpb2);
  //printf("path %s\n", path);
  
  // Проверка на попытки взлома
  if(strstr(path, "/../") != NULL)
  {
    send_answer(sockId, HTTP_400);
    status = 2;
  }

  int f = open(path, O_RDONLY);
  if((f != -1)&&(status == 0))
  {
    
    char *ext = strrchr(path, '.');
    ext++;
    
    char *handle = getCGIHandler(ext);
    //if(handle != NULL) printf("%s is CGI script, executed by %s\n",ext,handle);
    //else printf("%s is not CGI script\n",ext);
    
    if(handle != NULL)
    {
      //printf("We are GCI %s!\n", ext);
      close(f);
      
      //Now we need a pipe. Bidirectional. So...
      int pipefd[2][2];
      
      if(pipe(pipefd[0]) == -1)
      {
	printf("PIPE() error 1!\n");
	return 1;
      }
      
      if(pipe(pipefd[1]) == -1)
      {
	printf("PIPE() error 2!\n");
	return 1;
      }
      
      if(fork() == 0)
      {	//Мы в дочернем процессе
	//Закроем два ненужных дескриптора
	close(0);
	close(1);
      
	//Закроем концы pipe'ов
	close(pipefd[0][0]);
	close(pipefd[1][1]);
	
	//Теперь переназначим стандартные дескрипторы
	dup2(pipefd[1][0], 0);	//stdin
	dup2(pipefd[0][1], 1);	//stdout
	
	// Устанавливаем окружение
	clearenv();
	setenv("HTTP_HOST", in->host, 1);
	setenv("HTTP_REFERER", in->referer, 1);
	setenv("HTTP_ACCEPT", in->accept, 1);
	setenv("HTTP_USER_AGENT", in->uagent, 1);
	setenv("QUERY_STRING", in->args, 1);
	setenv("SCRIPT_FILENAME", path, 1);
	setenv("CONTENT_LENGTH", in->contlen, 1);
	setenv("HTTP_COOKIE", in->cookie, 1);
	setenv("CONTENT_TYPE", in->conttype, 1);
	setenv("SERVER_SOFTWARE", "MPID/0.0.1", 1);
	setenv("SERVER_NAME", cfg.sname, 1);
	setenv("REMOTE_ADDR", clientIp, 1);
	setenv("REMOTE_PORT", port, 1);
	
	if(in->method == METHOD_GET) setenv("REQUEST_METHOD", "GET", 1);
	if(in->method == METHOD_POST) setenv("REQUEST_METHOD", "POST", 1);
	
	//А теперь запускаем CGI
	if(strncmp(handle, HANDLE_INTERNAL, STR_LIMIT) == 0) execl(path, basename(path), NULL);	// Скрипт - сам себе голова
	else execl(handle, basename(handle), NULL); // Нужен обработчик
	exit(0);
      } else {
	//Родительский процесс
	close(pipefd[0][1]);
	close(pipefd[1][0]);

	if(in->method == METHOD_POST) 
	{
	  //printf("Written %d\n", write(pipefd[1][1], in->postdata, atoi(in->contlen)));
	  write(pipefd[1][1], in->postdata, atoi(in->contlen));
	}
	close(pipefd[1][1]);
	f = pipefd[0][0];
        //printf("alive2\n");
	/*memset(tmpbuf, '\0', 16384);
	  
	length = 16384;
    
	do {
	  //length = fread(tmpbuf, 1, 16384, f);
	  length = read(pipefd[0][0], tmpbuf, 16384);
	  //printf("%d\n", length);
	  send(sockId, tmpbuf, length, (int)NULL);
	} while (length != 0);
      //pclose(f);
      close(pipefd[0][0]);*/
      
      }
      //printf("CGI done\n");
      //printf("|%s|\n",in->conttype);
    } /*else {
      sprintf(answer, "Content-type: %s\r\n", get_content_type(ext));
      send(sockId, answer, strlen(answer), (int)NULL);
    
      lseek(f, 0, SEEK_SET);
      length = lseek(f, 0, SEEK_END);
      lseek(f, 0, SEEK_SET);
    
      sprintf(answer, "Content-Length: %d\r\n\r\n", length);
      send(sockId, answer, strlen(answer), (int)NULL);
    
      /*memset(tmpbuf, '\0', 16384);
      
      length = 16384;
    
      do {
	length = read(f, tmpbuf, 16384);
	//printf("Sent: %d\n", length);
	send(sockId, tmpbuf, length, (int)NULL);
      } while (length != 0);
    
      close(f);* /
    }*/
    
      free(tmpbuf);
      
      tmpbuf = NULL;
      int tmpsize = 0;

 /* length = 64*1024;
  do {
    tmpbuf = realloc(tmpbuf, tmpsize+64*1024);
    memset(tmpbuf+tmpsize, '\0', 64*1024);
    length = recv(sockId, tmpbuf+tmpsize, 64*1024, (int)NULL/*MSG_WAITALL* /);
    tmpsize += length;
  } while (length == 64*1024);*/
  
      length = 16384;
    
      do {
	tmpbuf = realloc(tmpbuf, tmpsize+16384);
	memset(tmpbuf+tmpsize, '\0', 16384);
	length = read(f, tmpbuf+tmpsize, 16384);
	tmpsize += length;
	//printf("Sent: %d\n", length);
	//send(sockId, tmpbuf, length, (int)NULL);
      } while (length != 0);
      
      if(handle != NULL)
      {
	// По стандарту CGI мы должны отфильтровать вывод скрипта.
	// Выбираются три заголовка:
	// Status: 404 Not Found - код ответа, который бы хотел видеть скрипт
	// Location: http://hdfjkhg.ru/uuuu/oooo.php - Переадресация (необходимо послать ответ 302 Moved Permanently)
	// Content-type: text/html - Бред какой-то, зачем его парсить?
	// Заголовки, по идее, надо вырезать, но неохота же =)
	// Можно их просто полпортить до нечитаемого браузерои вида
	char *temp = tmpbuf;
	printf("Origin:\n+++%s+++\n", tmpbuf);
	while((temp[0] != 0)&&(temp[0] != '\r')&&(temp[0] != '\n'))
	{	  
	  if(strncasecmp(temp, "Status: ", 8) == 0)
	  {
	    //printf("Found status header\n");
	    temp[0] = 'X';
	    temp[1] = '-';
	    while((temp[0] != ' ')&&(temp[0] != '\0')) *temp++;
	    send_answer(sockId, atoi(temp));
	    status = 3;
	    continue;
	  }
	  
	  if(strncasecmp(temp, "Location: ", 10) == 0)
	  {
	    printf("Found location header\n");
	    if((temp[10] != 'h')&&(temp[10] != '/'))	//Если адрес не абсолютный и не URL, нужна коррекция
	    {
	      printf("|||||||||||||||||||%s||||||||||||||\n",tmpbuf);
	      printf("|||||||||||||||||||%s||||||||||||||\n",temp);
	      char *relpath = dirname(strstr(path, in->fname));	// Относительный путь
	      strcat(relpath, "/");
	      printf("relpath %s\n", relpath);
	      char *newbuf = malloc(tmpsize+2);
	      temp[8] = '%';
	      temp[9] = 's';
	      sprintf(newbuf, tmpbuf, ": %s");
	      printf("|||||||||||||||||||%s||||||||||||||\n",newbuf);
	      tmpsize = tmpsize + strlen(relpath) + 1;
	      tmpbuf = realloc(tmpbuf, tmpsize);
	      sprintf(tmpbuf, newbuf, relpath);
	      free(newbuf);
	      printf("|||||||||||||||||||%s||||||||||||||\n",tmpbuf);
	      if(status == 0)
	      {
		send_answer(sockId, HTTP_302);
		status = 4;
	      }
	    }
	  }
	  
	  while((temp[0] != 0)&&(temp[0] != '\n')) *temp++;
	  if(temp[0] != 0) *temp++;
	}
	  if(status == 0) send_answer(sockId, HTTP_200);
      } else {
	send_answer(sockId, HTTP_200);
	sprintf(answer, "Content-type: %s\r\n", get_content_type(ext));
	send(sockId, answer, strlen(answer), (long)NULL);
    
	lseek(f, 0, SEEK_SET);
	length = lseek(f, 0, SEEK_END);
	lseek(f, 0, SEEK_SET);
    
	sprintf(answer, "Content-Length: %d\r\n\r\n", length);
	send(sockId, answer, strlen(answer), (long)NULL);
      }
      close(f);
      
      send(sockId, tmpbuf, tmpsize, (long)NULL);
  } else {
    if(!status)
    {
      //printf("Not found!\n");
      send_answer(sockId, HTTP_404);
      //printf("Error done\n");
    }
  } 
  
  printf("Job done\n\n");
  
  free(tmpbuf);
  //free(args);
  free(path);
  free(answer);
  //free(request);

  free(in);
  
  //close(sockIdNew);
  return CONN_NEED_CLOSE;
}
