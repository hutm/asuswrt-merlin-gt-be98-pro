#include <stdint.h>
#include <string.h>
extern void dbg(const char *,...);
extern int cprintf(const char *,...);
extern int get_model(void);
void dumpWfiTag(const void *tag)
{
    if(!tag)return;
    uint32_t fields[5];memcpy(fields,tag,sizeof(fields));
    dbg("WFI tag:\n");
    dbg("  CRC:        0x%08x\n",fields[0]);
    dbg("  Version:    0x%08x\n",fields[1]);
    dbg("  Chip ID:    0x%08x\n",fields[2]);
    dbg("  Flash Type: 0x%08x\n",fields[3]);
    dbg("  Flags:      0x%08x\n",fields[4]);
}
int check_imagefile(char *filename)
{
    (void)get_model();
    cprintf("%s, skip chk [%s]\n","check_imagefile",filename?filename:"");
    return 0;
}
int fw_check(void){return 0;}
