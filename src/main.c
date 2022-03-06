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

OS_RELEASE parseOSRELEASE(char *osRFile, size_t size); // parse the /etc/os-release file if it wasn't obvious.

size_t strsize(char *str); // get the size of a string str

int main(int argc, char **argv) {
  char user[8] = "user";

  char *err = "no error message set"; // default error message

  // uname
  uname(&uts);

  // files
  //   /etc/os-release
  FILE *osReleaseFile = fopen("./os-release", "r");
  struct stat osRStat;
  stat("/etc/os-release", &osRStat);
  char *osRelease = malloc(osRStat.st_size*sizeof(char));
  fread(osRelease, sizeof(char), osRStat.st_size, osReleaseFile);
  OS_RELEASE release = parseOSRELEASE(osRelease, osRStat.st_size);
  free(osRelease);

  // username
  getlogin_r(user, 8);

  printf("\033[1;31m%s\033[1;30m@\033[1;31m%s\n\033[0;0m", user, uts.nodename);
  printf("\033[0;38m-----------\n");
  printf("\033[1;31mOS\033[1;0m: %s %s\n", release.PRETTY_NAME, uts.machine);
  printf("\033[1;31mKernel\033[1;0m: %s\n", uts.release);

  printf("\n");
  return 0;

  ERROR: {
    fprintf(stderr, "err: %s\n", err);
    return -1;
  }
}

OS_RELEASE parseOSRELEASE(char *osRStr, size_t size) {
  OS_RELEASE ret = {"Linux", NULL, "linux", "linux-logo"}; // default return values

  char *nameStr = calloc(1, 64);
  char *valStr = calloc(1, 128);

  bool c; // whether the value is comma wraped or not

  for (int p = 0; p < size; p++) {
    for (uint8_t i = 0; i < 64; i++) nameStr[i] = 0; // zero out nameStr
    for (uint8_t i = 0; i < 128; i++) valStr[i] = 0; // zero out valStr

    for (int np = 0; osRStr[p] != '='; np++) { // get name
      nameStr[np] = osRStr[p];
      p++;
    }

    if (osRStr[p] == '"') c = true; // check for comma wrap

    p++;

    for (int np = 0; osRStr[p] != '\n'; np++) { // get value
      valStr[np] = osRStr[p];
      p++;
    }

    if (c) { // remove comma
      valStr++; 
      valStr[strsize(valStr) - 1] = 0;
    }

    // check for what to assign valStr to
    if (strcmp(nameStr, "NAME") == 0) {
      ret.NAME = malloc(strsize(valStr));
      strcpy(ret.NAME, valStr);
    } else if (strcmp(nameStr, "PRETTY_NAME") == 0) {
      ret.PRETTY_NAME = malloc(strsize(valStr));
      strcpy(ret.PRETTY_NAME, valStr);
    } else if (strcmp(nameStr, "ID") == 0) {
      ret.ID = malloc(strsize(valStr));
      strcpy(ret.ID, valStr);
    } else if (strcmp(nameStr, "LOGO") == 0) {
      ret.LOGO = malloc(strsize(valStr));
      strcpy(ret.LOGO, valStr);
    }
  }

  if (ret.PRETTY_NAME == NULL) ret.PRETTY_NAME = ret.NAME; // set PRETTY_NAME to NAME if PRETTY_NAME is still NULL (not defined in /etc/os-release)

  return ret;
}

char **parseConfig(FILE *confFile) {

}

size_t strsize(char *str) {
  int i = 0;
  for (; str[i]; i++) {} // find length (anything above 0 is truthy)
  return i;
}