#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/utsname.h>
#include <sys/stat.h>

typedef struct {
  char *NAME;
  char *PRETTY_NAME;
  char *ID;
  char *LOGO;
} OS_RELEASE;

typedef struct {
  int total;
  int avail;
  int cached;
} MEMINFO;

void color(); // print color blocks

OS_RELEASE parseOSRELEASE(char *osRFile, size_t size); // parse the /etc/os-release file if it wasn't obvious.

MEMINFO parseMemInfo(char *meminfoStr, size_t size); // parse the /proc/meminfo file if it wasn't obvious.

int main(int argc, char **argv) {
  char user[8] = "user";

  struct utsname uts;

  char *err = "no error message set"; // default error message

  // files
  //   /etc/os-release
  int osReleaseFile = open("/etc/os-release", O_RDONLY);
  if (osReleaseFile == -1) {
    err = "/etc/os-release";
    goto ERROR;
  } 
  struct stat osRStat;
  stat("/etc/os-release", &osRStat);
  char *osRelease = mmap(NULL, osRStat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, osReleaseFile, 0);
  OS_RELEASE release = parseOSRELEASE(osRelease, osRStat.st_size);
  close(osReleaseFile);

  //   /proc/meminfo
  FILE *memInfoFile = fopen("/proc/meminfo", "r");
  struct stat mIStat;
  stat("/etc/os-release", &mIStat);
  char *memInfoStr = malloc(mIStat.st_size*sizeof(char));
  fread(memInfoStr, sizeof(char), mIStat.st_size, memInfoFile);
  MEMINFO memInfo = parseMemInfo(memInfoStr, mIStat.st_size);
  fclose(memInfoFile);
  free(memInfoStr);
  
  // uname
  uname(&uts);

  // username
  getlogin_r(user, 8);

  // print info
  printf("\e[1;31m%s\e[1;90m@\e[1;31m%s\n\e[0;0m", user, uts.nodename);
  printf("\e[1;90m-----------\n");
  printf("\e[1;31mOS\e[1;0m: %s %s\n", release.PRETTY_NAME, uts.machine);
  printf("\e[1;31mKernel\e[1;0m: %s\n", uts.release);
  printf("\e[1;31mMemory\e[1;0m: %dMiB / %dMiB\n", memInfo.total / 1024 - memInfo.avail / 1024, memInfo.total / 1024);

  color();

  printf("\n");

  return 0;

  ERROR: {
    fprintf(stderr, "%s: %s\n", err, strerror(errno));
    return -1;
  }
}

void color() {
  printf("\e[0m\n");

  for (uint8_t i = 0; i < 8; i++) { // first row of colors
    printf("\e[0;4%dm   ", i);
  }

  printf("\e[0m\n");

  for (uint8_t i = 0; i < 8; i++) { // second row of colors
    printf("\e[0;10%dm   ", i);
  }

  printf("\e[0m\n");
}

OS_RELEASE parseOSRELEASE(char *osRStr, size_t size) {
  OS_RELEASE ret = {"Linux", NULL, "linux", "linux-logo"}; // default return values

  char *nameStr = calloc(1, 64);
  char *valStr = calloc(1, 128);

  bool c; // whether the value is comma wraped or not

  for (uint16_t p = 0; p < size; p++) {
    for (uint8_t i = 0; i < 64; i++) nameStr[i] = 0; // zero out nameStr
    for (uint8_t i = 0; i < 128; i++) valStr[i] = 0; // zero out valStr

    for (int np = 0; osRStr[p] != '='; np++) { // get name
      nameStr[np] = osRStr[p];
      p++;
    }

    p++;

    if (osRStr[p] == '\"') c = true; // check for comma wrap

    for (int np = 0; osRStr[p] != '\n'; np++) { // get value
      valStr[np] = osRStr[p];
      p++;
    }

    if (c) { // remove comma
      valStr++;
      valStr[strlen(valStr) - 1] = 0;
    }

    // check for what to assign valStr to
    if (strcmp(nameStr, "NAME") == 0) {
      ret.NAME = malloc(strlen(valStr));
      strcpy(ret.NAME, valStr);
    } else if (strcmp(nameStr, "PRETTY_NAME") == 0) {
      ret.PRETTY_NAME = malloc(strlen(valStr));
      strcpy(ret.PRETTY_NAME, valStr);
    } else if (strcmp(nameStr, "ID") == 0) {
      ret.ID = malloc(strlen(valStr));
      strcpy(ret.ID, valStr);
    } else if (strcmp(nameStr, "LOGO") == 0) {
      ret.LOGO = malloc(strlen(valStr));
      strcpy(ret.LOGO, valStr);
    }
  }

  if (ret.PRETTY_NAME == NULL) ret.PRETTY_NAME = ret.NAME; // set PRETTY_NAME to NAME if PRETTY_NAME is still NULL (not defined in /etc/os-release)

  return ret;
}

MEMINFO parseMemInfo(char *memInfoStr, size_t size) {
  MEMINFO ret;

  char *nameStr = calloc(1, 64);
  char *valStr = calloc(1, 128);

  bool c; // whether the value is comma wraped or not

  for (uint16_t p = 0; p < size; p++) {
    for (uint8_t i = 0; i < 64; i++) nameStr[i] = 0; // zero out nameStr
    for (uint8_t i = 0; i < 128; i++) valStr[i] = 0; // zero out valStr

    for (int np = 0; memInfoStr[p] != ':'; np++) { // get name
      nameStr[np] = memInfoStr[p];
      p++;
    }

    while (memInfoStr[p] == ' ') {p++;}
    p++;

    for (int np = 0; memInfoStr[p] != '\n'; np++) { // get value
      valStr[np] = memInfoStr[p];
      p++;
    }

    if (c) { // remove comma
      valStr++;
      valStr[strlen(valStr) - 1] = 0;
    }

    // check for what to assign valStr to
    if (strcmp(nameStr, "MemTotal") == 0) {
      ret.total = atoi(valStr);
    } else if (strcmp(nameStr, "MemAvailable") == 0) {
      ret.avail = atoi(valStr);
    } else if (strcmp(nameStr, "Cached") == 0) {
      ret.cached = atoi(valStr);
    }
  }

  return ret;
}