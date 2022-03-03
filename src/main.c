#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "bool.h"

#include <sys/utsname.h>
#include <sys/stat.h>

struct utsname uts;

typedef struct {
  char *NAME;
  char *PRETTY_NAME;
  char *ID;
  char *LOGO;
} OS_RELEASE;

OS_RELEASE parseOSRELEASE(char *osRFile, struct stat osRSt);

char user[8] = "user";
char *err = "no error message set";
size_t strsize(char *str);

int main(int argc, char **argv) {
  // uname
  uname(&uts);

  // files
  //   /etc/os-release
  FILE *osReleaseFile = fopen("/etc/os-release", "r");
  struct stat osRStat;
  stat("/etc/os-release", &osRStat);
  char *osRelease = malloc(osRStat.st_size*sizeof(char));
  fread(osRelease, sizeof(char), osRStat.st_size, osReleaseFile);
  OS_RELEASE release = parseOSRELEASE(osRelease, osRStat);

  // username
  getlogin_r(user, 8);

  printf("\033[1;31m%s\033[0;0m@\033[1;31m%s\n\033[0;0m", user, uts.nodename);
  printf("\033[0;0m-----------\n");
  printf("\033[1;31mOS\033[1;0m: %s %s\n", release.PRETTY_NAME, uts.machine);
  printf("\033[1;31mKernel\033[1;0m: %s\n", uts.release);

  printf("\n");
  return 0;

  ERROR: {
    printf("err: %s\n", err);
    return -1;
  }
}

OS_RELEASE parseOSRELEASE(char *osRStr, struct stat osRSt) {
  OS_RELEASE ret = {"Linux", NULL, "linux", NULL};

  char *nameStr = malloc(64);
  char *valStr = malloc(128);
  bool c;
  for (int p = 0; p < osRSt.st_size; p++) {
    for (uint8_t i = 0; i < 128; i++) valStr[i] = 0;
    for (int np = 0; osRStr[p] != '='; np++) {
      nameStr[np] = osRStr[p];
      p++;
    }
    if (osRStr[p] == '\"') {
      c = true;
    }
    p++;
    for (int np = 0; osRStr[p] != '\n'; np++) {
      valStr[np] = osRStr[p];
      p++;
    }

    if (c) valStr++;

    valStr[strsize(valStr) - 1] = 0;

    if (strcmp(nameStr, "NAME")) {
      ret.NAME = malloc(strsize(valStr));
      strcpy(ret.NAME, valStr);
    } else if (strcmp(nameStr, "PRETTY_NAME")) {
      ret.PRETTY_NAME = malloc(strsize(valStr));
      strcpy(ret.PRETTY_NAME, valStr);
    } else if (strcmp(nameStr, "ID")) {
      ret.ID = malloc(strsize(valStr));
      strcpy(ret.ID, valStr);
    } else if (strcmp(nameStr, "LOGO")) {
      ret.LOGO = malloc(strsize(valStr));
      strcpy(ret.LOGO, valStr);
    }
  }

  if (ret.PRETTY_NAME == NULL) ret.PRETTY_NAME = ret.NAME;
  if (ret.LOGO == NULL) ret.LOGO = "generic-logo";

  return ret;
}



char **parseConfig(FILE *confFile) {

}

size_t strsize(char *str) {
  int i = 0;
  for (; str[i]; i++) {}
  return i;
}