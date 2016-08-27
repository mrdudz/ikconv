
#define MAXTEXTLEN 0xffff
#define MAXNAMELEN 0xff

char karaoke_text[MAXTEXTLEN];
char karaoke_songname[MAXNAMELEN];
char karaoke_author[MAXNAMELEN];

typedef struct {
    int format; /* 0xff: end of events 0 = min:sec:.. 1: 50hz frames */
    int frames;
    int min, sec, hsec;
    unsigned int textoffs; /* offset into text */
} KARAOKETIMING;

KARAOKETIMING karaoke_timing[MAXTEXTLEN];
unsigned int karoke_events = 0;

#define KT_FORMAT_END           0xff
#define KT_FORMAT_MINSEC        0x00
#define KT_FORMAT_FRAMES        0x01
