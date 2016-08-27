
static float bpm = 300;
static unsigned int initial_delay = 0; /* initial delay in milliseconds */

static char mp3name[0x100];

//------------------------------------------------------------------------------

static int beatstoframes(int beats)
{
//    unsigned int frames = ((3007.5f * (float)beats) / (float)bpm);
    unsigned int frames = (((3007.5f / 4.0f) * (float)beats) / (float)bpm);
    return frames;
}

static int framestobeats(int frames)
{
    unsigned int beats = ((float)frames * (float)bpm / (3007.5f / 4.0f));
    return beats;
}

//------------------------------------------------------------------------------

int writeusc(char *name)
{
    int x,y,len;
    FILE *f = fopen(name, "wb");
    unsigned int events = 0, frames = 0, delay_frames = 0;
    char linebuf[0x100];

    if (f == NULL) {
        return -1;
    }

    delay_frames = karaoke_timing[0].frames;
    initial_delay = delay_frames * 20;

    fprintf(f, "#TITLE:%s\n", karaoke_songname);
    fprintf(f, "#ARTIST:%s\n", karaoke_author);
    fprintf(f, "#MP3:%s\n", mp3name);
    fprintf(f, "#BPM:%2.2f\n", bpm);
    fprintf(f, "#GAP:%d\n", initial_delay);

    while (karaoke_timing[events].format != 0xff) {
        if (karaoke_timing[events + 1].format != 0xff) {
            len = karaoke_timing[events+1].textoffs - karaoke_timing[events].textoffs;
//        len = 10;
            if (len < 0) {
                len = 0;
                fprintf(stderr, "WARNING: length of event is negative (text offset: %d\n", karaoke_timing[events].textoffs);
            }
            strncpy(linebuf, &karaoke_text[karaoke_timing[events].textoffs], len);
//        strncpy(linebuf, &karaoke_text[karaoke_timing[0].textoffs], 0x20);
        } else {
            strncpy(linebuf, &karaoke_text[karaoke_timing[events].textoffs],0xff);
            len = strlen(linebuf);
        }
        linebuf[len]=0;
        x = 1; // FIXME
        y = 0; // FIXME
        frames = framestobeats(karaoke_timing[events].frames - delay_frames);
        if (linebuf[0] == '\n') {
            //fprintf(f, "-%d %d %d\n", karaoke_timing[events].frames, x, y);
            fprintf(f, "- %d\n", frames);
            if (strlen(&linebuf[1]) > 0 ) {
                fprintf(f, ": %d %d %d %s\n", frames, x, y, &linebuf[1]);
            }
        } else if (linebuf[len-1] == '\n') {
            linebuf[len-1] = 0;
            if (strlen(linebuf) > 0) {
                fprintf(f, ": %d %d %d %s\n", frames, x, y, linebuf);
            }
            //fprintf(f, "-%d %d %d\n", frames, x, y);
            fprintf(f, "- %d\n", frames);
        } else {
            fprintf(f, ": %d %d %d %s\n", frames, x, y, linebuf);
        }
        events++;
    }
    fprintf(f, "E\n");

    fclose(f);
    return 0;
}

//------------------------------------------------------------------------------

int readusc(char *name)
{
    FILE *f = fopen(name, "rb");
    unsigned int c, istag, tagptr, lineptr, textptr, timingptr, beats,x,y;
    char tagbuf[0x100];
    char linebuf[0x100];
    unsigned int delay_frames = 0;

    if (f == NULL) {
        return -1;
    }

    karoke_events=0;
    karaoke_text[0] = 0;
    karaoke_timing[0].format = 0xff;
    textptr = 0;
    timingptr = 0;
    while (!feof(f)) {
        c = fgetc(f);
        switch(c) {
            case '#':
                fscanf(f, "%[^:]:%[^\n]\n", tagbuf, linebuf);
                if (verbose > 1) printf("tag:%s tagarg:%s\n", tagbuf, linebuf);
                if (!strcmp("GAP", tagbuf)) {
                     /* initial delay in milliseconds */
                     initial_delay = atoi(linebuf);
                     delay_frames = initial_delay / 20;
                } else if (!strcmp("BPM", tagbuf)) {
                     bpm = atoi(linebuf); // FIXME
                } else if (!strcmp("TITLE", tagbuf)) {
                    strcpy(karaoke_songname, linebuf);
                } else if (!strcmp("ARTIST", tagbuf)) {
                    strcpy(karaoke_author, linebuf);
                } else if (!strcmp("MP3", tagbuf)) {
                    strcpy(mp3name, linebuf);
                }
                break;
            case ':':
                fscanf(f, "%d %d %d%[^\n]\n", &beats, &x, &y, linebuf);
                if (verbose > 1) printf("line: %d beats, %d, %d, >%s<\n", beats, x, y, linebuf+1);
                strcat(&karaoke_text[textptr], linebuf+1);
                karaoke_timing[timingptr].format = 1;
                karaoke_timing[timingptr].frames = beatstoframes(beats) + delay_frames;
                karaoke_timing[timingptr].textoffs = textptr;
                textptr+=strlen(linebuf+1);
                timingptr++;
                karoke_events++;
                break;
            case '-':
                fscanf(f, "%d\n", &beats);
                if (verbose > 1) printf("linebreak:%d beats\n", beats);
                strcat(&karaoke_text[textptr], "\n");
                karaoke_timing[timingptr].format = 1;
                karaoke_timing[timingptr].frames = beatstoframes(beats) + delay_frames;
                karaoke_timing[timingptr].textoffs = textptr;
                textptr+=1;
                timingptr++;
                karoke_events++;
                break;
            case 'E':
                karaoke_timing[timingptr].format = 0xff;
                karaoke_timing[timingptr].frames = 0;
                karaoke_timing[timingptr].textoffs = textptr;
                break;
        }
    }
    if (verbose) printf("found %d events in file.\n", karoke_events);

    fclose(f);
    return 0;
}
