#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void accstrcpy(char **dstAcc, const char *src)
{
  while (*((*dstAcc)++) = *(src++));
  /* Start next copy on top of NUL */
  --(*dstAcc);
}

int main(int argc, char **argv)
{
  if (argc == 0)
  {
    return -1;
  }

  char *const dst = (char *)malloc(strlen(argv[0]) + 5);
  if (!dst)
  {
    return -1;
  }
  char *dstAcc = dst;
  accstrcpy(&dstAcc, argv[0]);
  accstrcpy(&dstAcc, ".dst");
  FILE *pDstFile = fopen(dst, "rb");
  if (!pDstFile)
  {
    free(dst);
    return -1;
  }

  fseek(pDstFile, 0, SEEK_END);
  int dstFileLen = ftell(pDstFile);
  rewind(pDstFile);

  int totalLen = 1 + dstFileLen;
  for (int i = 1; i < argc; ++i)
  {
    /* Add 1 for space after each argument and NUL after last */
    totalLen += strlen(argv[i]) + 1;
  }

  char *const cmd = (char *)malloc(totalLen);
  if (!cmd)
  {
    free(dst);
    fclose(pDstFile);
    return -1;
  }
  fread(cmd, 1, dstFileLen, pDstFile);
  fclose(pDstFile);
  char *cmdAcc = cmd + dstFileLen;
  /* Will be overwritten if any arguments */
  *cmdAcc = '\0';

  for (int i = 1; i < argc; ++i)
  {
    *(cmdAcc++) = ' ';
    accstrcpy(&cmdAcc, argv[i]);
  }

  int result = system(cmd);
  free(dst);
  free(cmd);

  return result;
}
