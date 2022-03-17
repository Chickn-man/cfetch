#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#include <sys/mman.h>
#include <sys/utsname.h>
#include <sys/stat.h>

#include "ansicolors.h"
#include "../config.h"

typedef struct {
  char *NAME;
  char *PRETTY_NAME;
  char *ID;
  char *LOGO;
} OS_RELEASE;

typedef struct {
  int total;
  int free;
  int buffers;
  int cached;
  int shmem;
  int srec;
} MEMINFO;

void die(char *msg); // print the string at msg, a colon, then the string associated with errno.

char *colors[6];
void parseColors(); // make colors array based off of colorConf

void info(char *fmt, ...); // printf with special stuff;

void infoCols(); // print color blocks

OS_RELEASE parseOSRELEASE(char *osRFile, size_t size); // parse the /etc/os-release file if it wasn't obvious.

MEMINFO parseMemInfo(char *meminfoStr, size_t size); // parse the /proc/meminfo file if it wasn't obvious.

int main(int argc, char **argv) {
  char user[8] = "user";

  struct utsname uts; 

  char *err = "no error message set"; // default error message

  // files
  //   /etc/os-release
  int osReleaseFile = open("/etc/os-release", O_RDONLY);
  if (osReleaseFile == -1) die("/etc/os-release");
  struct stat osRStat;
  fstat(osReleaseFile, &osRStat);
  char *osRelease = mmap(NULL, osRStat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, osReleaseFile, 0);
  OS_RELEASE release = parseOSRELEASE(osRelease, osRStat.st_size);
  close(osReleaseFile);

  //   /proc/meminfo
  FILE *memInfoFile = fopen("/proc/meminfo", "r");
  struct stat mIStat;
  stat("/proc/meminfo", &mIStat);
  mIStat.st_size = 712;
  char *memInfoStr = calloc(1, mIStat.st_size*sizeof(char));
  fread(memInfoStr, sizeof(char), mIStat.st_size, memInfoFile);
  MEMINFO memInfo = parseMemInfo(memInfoStr, mIStat.st_size);
  fclose(memInfoFile);
  free(memInfoStr);
  
  // uname
  uname(&uts);

  // username
  getlogin_r(user, 8);

  parseColors();

  // finaly print the shit
  printf("%s%s%s@%s%s\n\e[0;0m", colors[0], user, colors[1], colors[0], uts.nodename);
  printf("%s-----------\n", colors[2]);
  info("OS %s %s", release.PRETTY_NAME, uts.machine);
  info("Kernel %s", uts.release);
  info("Memory %dMiB / %dMiB",
  (memInfo.total + memInfo.shmem - memInfo.free - memInfo.buffers - memInfo.cached - memInfo.srec) / 1024,
  memInfo.total / 1024
  );

  infoCols();

  printf("\n");

  return 0;

  ERROR: {
    die(err);
  }
}

/*
** print the string at msg, a colon, then the string associated with errno.
*/
void die(char *msg) {
  printf("%s: %s\n", msg, strerror(errno));
  exit(errno);
}

/* 
** make colors array based off of colorConf
*/
void parseColors() {
  for (int i = 0; i < 6; i++) {
    if (i == 2 || i == 4 || i == 5) {
      colors[i] = ansiColors[colorConf[i]];
      continue;
    }
    if (bold) colors[i] = ansiColorsBold[colorConf[i]]; else colors[i] = ansiColors[colorConf[i]];
  }
}

/* 
** printf with special stuff;
*/
void info(char *fmt, ...) {
  fputs(colors[3], stdout);
  int i;
  for (i = 0; fmt[i] != ' '; i++) {putc(fmt[i], stdout);}

  fputs(colors[4], stdout);
  putchar(':');

  va_list ap;
  va_start(ap, fmt);

  fputs(colors[5], stdout);
  for (char *p = fmt + i; *p; p++) {
    if (*p != '%') {
      putchar(*p);
      continue;
    }
    switch (*++p) {
      case 's':
        printf(va_arg(ap, char*));
        break;
      case 'd':
        printf("%d", va_arg(ap, int));
        break;
      default:
        putchar(*p);
        break;
    }
  }

  putchar('\n');
}

/*
** print color blocks
*/
void infoCols() {
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

  char *nameStr = malloc(64);
  char *valStr = malloc(128);

  bool c; // whether the value is comma wraped or not

  for (uint16_t p = 0; p < size; p++) {
    for (uint8_t i = 0; i < 64; i++) nameStr[i] = 0; // zero out nameStr
    for (uint8_t i = 1; i < 128; i++) valStr[i] = 0; // zero out valStr

    for (int np = 0; osRStr[p] != '='; np++) { // get name
      nameStr[np] = osRStr[p];
      p++;
    }

    p++; // increment p

    if (osRStr[p] == '\"') c = true; // check for comma wrap

    for (int np = 0; osRStr[p] != '\n'; np++) { // get value
      valStr[np] = osRStr[p];
      p++;
    }

    valStr[strlen(valStr)] = 0;

    if (c) { // remove comma
      valStr++;
      valStr[strlen(valStr) - 1] = 0;
    }

    // check for what to assign valStr to
    if (strcmp(nameStr, "NAME") == 0) {
      ret.NAME = malloc(strlen(valStr) + 1);
      strcpy(ret.NAME, valStr);
    } else if (strcmp(nameStr, "PRETTY_NAME") == 0) {
      ret.PRETTY_NAME = malloc(strlen(valStr) + 1);
      strcpy(ret.PRETTY_NAME, valStr);
    } else if (strcmp(nameStr, "ID") == 0) {
      ret.ID = malloc(strlen(valStr) + 1);
      strcpy(ret.ID, valStr);
    } else if (strcmp(nameStr, "LOGO") == 0) {
      ret.LOGO = malloc(strlen(valStr) + 1);
      strcpy(ret.LOGO, valStr);
    }
  }

  if (ret.PRETTY_NAME == NULL) ret.PRETTY_NAME = ret.NAME; // set PRETTY_NAME to NAME if PRETTY_NAME is still NULL (not defined in /etc/os-release)

  return ret;
}

MEMINFO parseMemInfo(char *memInfoStr, size_t size) {
  MEMINFO ret = {0, 0, 0, 0, 0, 0};

  char *nameStr = calloc(1, 64);
  char *valStr = calloc(1, 128);

  for (uint16_t p = 0; p < size; p++) {
    for (uint8_t i = 0; i < 64; i++) nameStr[i] = 0; // zero out nameStr
    for (uint8_t i = 0; i < 128; i++) valStr[i] = 0; // zero out valStr

    for (int np = 0; memInfoStr[p] != ':'; np++) { // get name
      if (memInfoStr[p] == 0) return ret;
      nameStr[np] = memInfoStr[p];
      p++;
    }

    while (memInfoStr[p] == ' ') {p++; if (memInfoStr[p] == 0) return ret;}
    p++;

    for (int np = 0; memInfoStr[p] != '\n'; np++) { // get value
      if (memInfoStr[p] == 0) return ret;
      valStr[np] = memInfoStr[p];
      p++;
    }

    // check for what to assign valStr to
    if (strcmp(nameStr, "MemTotal") == 0) {
      ret.total = atoi(valStr);
    } else if (strcmp(nameStr, "MemFree") == 0) {
      ret.free = atoi(valStr);
    } else if (strcmp(nameStr, "Buffers") == 0) {
      ret.buffers = atoi(valStr);
    } else if (strcmp(nameStr, "Cached") == 0) {
      ret.cached = atoi(valStr);
    } else if (strcmp(nameStr, "Shmem") == 0) {
      ret.shmem = atoi(valStr);
    } else if (strcmp(nameStr, "SReclaimable") == 0) {
      ret.srec = atoi(valStr);
    }
  }

  return ret;
}