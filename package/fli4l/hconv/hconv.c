#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

/*
 * time_t
 * number
 */

typedef struct
{
  char *cmd;
  int (*print)(const char *arg);
} functable;

static int print_time_t(const char *arg);
static int print_hnum(const char *arg);

functable function_table[] = {
  { "time_t",     &print_time_t },
  { "hnum",       &print_hnum },
  { NULL,         NULL }
};

int main(int argc, char *argv[])
{
  int i;
  int ExitCode = 0;

  if(3 != argc)
    {
      fprintf(stderr, "usage is: todo\n");
      return (1);
    }
  
  for(i=0; function_table[i].cmd != NULL; i++)
    {
      if(0 == strcmp(argv[1], function_table[i].cmd))
        {
          ExitCode = function_table[i].print(argv[2]);
	}
    }
  return (ExitCode);
}

static int print_time_t(const char *arg)
{
  time_t tmp_timeVal;
  char buff[25];
  struct tm tmBuff;

  tmp_timeVal = (time_t)strtoll(arg, NULL, 0);
  localtime_r(&tmp_timeVal, &tmBuff);
  strftime(buff, sizeof(buff)-1, "%FT%T", &tmBuff);
  puts(buff);
  return (0);
}

static int print_hnum(const char *arg)
{
  signed long long int val;
 
  val = strtoll(arg, NULL, 0);
  if(val <= 1024LL)
    {
      printf("%lld\n", val);
    }
  else if(val <= 1024LL*1024LL)
    {
      printf("%.3f KB\n", (double)val/1024LL);
    }
  else if(val <= 1024LL*1024LL*1024LL)
    {
      printf("%.3f MB\n", (double)val/(1024LL*1024LL));
    }
  else
    {
      printf("%.3f GB\n", (double)val/(1024LL*1024LL*1024LL));
    }

  return (0);
}
