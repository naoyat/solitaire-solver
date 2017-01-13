#include <stdio.h>
int st[256];
main(){
  int c;
  while(1){
    c = getchar();
    if (c == EOF) break;
    st[c]++;
  }
  for (c=48;c<127;++c) if (st[c]>0) printf("%c:%d\n", c, st[c]);
}
