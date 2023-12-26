#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*
length := 0
read chunk-size, chunk-extension (if any) and CRLF
  while (chunk-size > 0) {
    read chunk-data and CRLF
    append chunk-data to entity-body
    length := length + chunk-size
    read chunk-size and CRLF
  }
read entity-header
while (entity-header not empty) {
  append entity-header to existing header fields
    read entity-header
    }
Content-Length := length
Remove "chunked" from Transfer-Encoding
*/

char buffer[2048];

int read_chunk (void)
{
  int len;
  char * err;
  int num=strtol(buffer, &err, 16);
  if (err == buffer)
    return -1;

  if (num == 0)
    return 0;

  while (fgets(buffer, sizeof(buffer)-1, stdin)) {
    len=strlen(buffer);
    if (len > num) {
      if (buffer[len - 2] == '\r' && buffer[len - 1] == '\n') {
        len-=2;
        buffer[len]=0;
      } else {
        printf("%s", buffer);
        return -1;
      }
    }
    printf("%s", buffer);
    num-=len;
    if (num <= 0)
      return 1;
  }
  return -1;
}
int main (void)
{
  int chunked = 1;
  while (fgets(buffer, sizeof(buffer)-1, stdin)) {
    if (chunked && isxdigit(buffer[0])) {
      if (!isalpha(buffer[0]) || islower(buffer[0])) {
        switch (read_chunk()) {
        case 0:
          chunked = 0;
        case 1:
          continue;
        case -1:
          chunked = 0;
        }
      }
    }
    printf("%s", buffer);
  }
  return 0;
}
