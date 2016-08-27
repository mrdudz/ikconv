
extern unsigned char sid_memory[0x10000];
extern unsigned int sid_data_addr;
extern unsigned int sid_data_length;
extern unsigned int sid_init_addr;
extern unsigned int sid_play_addr;

extern char sid_filename[0x100];

static unsigned int songname_offset;
static unsigned int author_offset;

static unsigned int music_speed;

static unsigned int data_length;
static unsigned int data_offset;

static unsigned int text_addr;
static unsigned int text_length;
static unsigned int timing_addr;
static unsigned int timing_length;

static int ikformat = 0;

int start_counter = 0;
int volume_addr = 0;
int fade_speed = 0x02;
int fade_delay = 0x0202;

static int writewithcr(FILE *f, unsigned char *s);

int events_skipped = 0;

//------------------------------------------------------------------------------

static unsigned int get16(FILE *f)
{
    unsigned int n = fgetc(f);
    n |= (fgetc(f) << 8);
    return n;
}

static int put16(FILE *f, int val)
{
    fputc(val & 0xff, f);
    return fputc((val >> 8) & 0xff, f);
}

static int writescreencode(FILE *f, char *text) {
    unsigned char c;
    unsigned int ptr = 0;
    if (ikformat == 0) {
        while((c = text[ptr])) {
            if (c == '\n') {
                c = 0;
            // crude conversion from ascii to uppercase screencode
            } else if ((c >= 'a') && (c <= 'z')){
                c -= ('a' - 1);
            } else if ((c >= 'A') && (c <= 'Z')){
                c -= ('A' - 1);
            }
            fputc(c, f);
            ptr++;
        }
    } else {
        fprintf(f, "    !scr \"");
        while((c = text[ptr])) {
            if (c == '\n') {
                // end of line
                fprintf(f, "\", 0\n");
                fprintf(f, "    !scr \"");
            } else {
                fprintf(f, "%c", tolower(c));
            }
            ptr++;
        }
        if (c != '\n') {
            fprintf(f, "\", 0\n");
        }
    }
    return 0;
}

//------------------------------------------------------------------------------

int readname(FILE *f)
{
    unsigned char c;
    unsigned int textptr = 0;
    while((c = fgetc(f)) != 0x00) {
        if ((c < 0x20))  {
            c += 'a' - 1;
        } else if ((c > 0x20))  {
            //c = 'X';
        }
        karaoke_songname[textptr] = c;
        textptr++;
    }
    karaoke_songname[textptr] = 0;
    return 0;
}

int readauthor(FILE *f)
{
    unsigned char c;
    unsigned int textptr = 0;
    while((c = fgetc(f)) != 0x00) {
        if ((c < 0x20))  {
            c += 'a' - 1;
        } else if ((c > 0x20))  {
            //c = 'X';
        }
        karaoke_author[textptr] = c;
        textptr++;
    }
    karaoke_author[textptr] = 0;
    return 0;
}

static int readsiddata(FILE *f)
{
    if (verbose) printf("readsiddata offset: %04x\n", data_offset);
    fseek(f, data_offset, SEEK_SET);
    fread(&sid_memory[sid_data_addr], sid_data_length, 1, f);
    return 0;
}

static int readtext(FILE *f)
{
    unsigned char c;
    unsigned int text_offset = (text_addr - sid_data_addr) + data_offset;
    unsigned int textptr = 0;
    if (verbose) printf("text offset: %04x\n", text_offset);
    fseek(f, text_offset, SEEK_SET);
    while((c = fgetc(f)) < 0xff) {
        if (c == 0) {
            c = '\n';
        } else if ((c < 0x20))  {
            c += 'a' - 1;
        } else if ((c > 0x20))  {
            //c = 'X';
        }
        karaoke_text[textptr] = c;
        textptr++;
    }
    karaoke_text[textptr] = 0;
    text_length = textptr;
    return 0;
}

static int readtiming(FILE *f)
{
    unsigned int timing_offset = (timing_addr - sid_data_addr) + data_offset;
//    unsigned int lastlineend = 0, lastlinepos = 0;
    unsigned int frames,x,y, linepos = 0, firstline;
    unsigned int timingptr = 0, textptr = 0, totalframes = 0;
    unsigned char linebuf[0x100];
    if (verbose) printf("timing offset: %04x\n", timing_offset);
    fseek(f, timing_offset, SEEK_SET);
    karoke_events=0;
    while(!feof(f)) {
        frames = (get16(f) - 0x100);
        x = fgetc(f); // offset in line
        y = fgetc(f); // number of chars to highlight

        if ((x == 0x00) && (y == 0xff)) {
            return 0;
        }
#if 1
        else if (x == 0x00) {
            //textptr += linepos + 1; /* skip LF */
            if (!firstline) {
    //           textptr += lastlineend + 1; /* skip LF */
               while (karaoke_text[textptr] != '\n') textptr++;
               //while (karaoke_text[textptr] == '\n') textptr++;
               textptr++;
           }
           firstline = 0;
            //printf(">>>%s<<<\n", &karaoke_text[textptr]);
        }
#endif
        y++;
        linepos = x + 0; // + y;
        
        if(linepos > 39) {
            fprintf(stderr,"internal error: (ikmod readtiming) linepos > 39 (=%d)\n", linepos);
        }
        
        if(y>=1) {
            strncpy(linebuf, &karaoke_text[linepos+ textptr], y);
            linebuf[y]=0;
        } else {
            printf("warning, highlight len is <1 (=%d)\n",y);
            y = 0;
            linebuf[y]=0;
        }
        
        if (verbose) {
            printf("%4d frames: %04x x: %02x y: %02x linepos: %2d textptr: %04x textoffs: %04x  text:", timingptr, frames, x, y,linepos , textptr, textptr + linepos);
            writewithcr(stdout, linebuf);
            printf("\n");
        }
        totalframes += frames;
        
        karaoke_timing[timingptr].format = 1;
        karaoke_timing[timingptr].frames = totalframes;
        karaoke_timing[timingptr].textoffs = linepos + textptr;

//        lastlineend = linepos + y;
//        lastlinepos = linepos;

        timingptr++;
        karaoke_timing[timingptr].format = 0xff;

        karoke_events++;
    }
    timing_length = timingptr * 4;
    return -1;
}

int readikmod(char *name)
{
    FILE *f = fopen(name, "rb");
    unsigned char c;

    if (f == NULL) {
        return -1;
    }

    /* $C9,$CB,$AB */
    c = fgetc(f);if (c != 0xc9) return -1;
    c = fgetc(f);if (c != 0xcb) return -1;
    c = fgetc(f);if (c != 0xab) return -1;

    songname_offset = get16(f);
    author_offset = get16(f);

    sid_init_addr = get16(f);
    sid_play_addr = get16(f);
    music_speed = fgetc(f);
    printf("init:%04x play:%04x speed:%d\n", sid_init_addr, sid_play_addr, music_speed);

    data_offset = get16(f);
    data_length = get16(f);
    sid_data_addr = get16(f);
    //sid_data_length = sid_data_lastaddr - sid_data_addr;
    printf("data offs: %04x len: %04x addr: %04x\n", data_offset, data_length, sid_data_addr);

    text_addr = get16(f);
    timing_addr = get16(f);
    printf("text addr: %04x timing addr: %04x\n", text_addr, timing_addr);

    readname(f);
    readauthor(f);

    readtext(f);
    if (verbose) printf("text:%s\n", karaoke_text);
    readtiming(f);

    sid_data_length = data_length;
    sid_data_length -= text_length;
    sid_data_length -= timing_length;
    printf("SID data len: %04x\n", sid_data_length);

    readsiddata(f);

    fclose(f);

    return 0;

}

//------------------------------------------------------------------------------

static int writename(FILE *f)
{
//    if (verbose) printf("song name: %s\n", karaoke_songname);
    writescreencode(f, karaoke_songname);
    fputc(0, f);
    return 0;
}

static int writeauthor(FILE *f)
{
//    if (verbose) printf("author name: %s\n", karaoke_author);
    writescreencode(f, karaoke_author);
    fputc(0, f);
    return 0;
}

static int writetext(FILE *f)
{
    unsigned int text_offset = (text_addr - sid_data_addr) + data_offset;
    if (verbose){
        printf("text offset: %04x\n", text_offset);
    }
    if (verbose > 1){
        printf("text: %s\n", karaoke_text);
    }
    fseek(f, text_offset, SEEK_SET);
    writescreencode(f, karaoke_text);
    fputc(0, f);
    fputc(0xff, f);
    return 0;
}

static int writesiddata(FILE *f)
{
    if (verbose) printf("writing sid_data_addr: %04x sid_data_length: %04x\n", sid_data_addr,sid_data_length);
    fwrite(&sid_memory[sid_data_addr], sid_data_length, 1, f);
    return 0;
}

static int writewithcr(FILE *f, unsigned char *s)
{
    while(*s) {
        if (*s == '\n') {
            fprintf(f, "<CR>");
        } else {
            fprintf(f, "%c", *s);
        }
        s++;
    }
    return 0;
}

static int writetiming(FILE *f)
{
    int frames, pos = 0, len, num = 0;
    unsigned int lastframes, textptr = 0;
    unsigned int timing_offset = (timing_addr - sid_data_addr) + data_offset;
    KARAOKETIMING *e = karaoke_timing;
    KARAOKETIMING *n = &karaoke_timing[1];
    unsigned char linebuf[0x100];
    lastframes = 0;
    if (verbose) printf("timing offset: %04x\n", timing_offset);
    events_skipped = 0;
    while(e->format != KT_FORMAT_END) {
#if 1
        if (karaoke_text[e->textoffs] == '\n') {
            textptr = n->textoffs;
            pos = 0;
            e++; n++;
            events_skipped++;
            continue;
        }
#endif
        len = n->textoffs - e->textoffs;
        frames = e->frames - lastframes;
        
        if(len>=1) {
            strncpy(linebuf, &karaoke_text[e->textoffs], len);
            linebuf[len]=0;
        } else {
            fprintf(stderr,"warning, text offset is <1 (=%d)\n",len);
            len = 1;
            linebuf[0]=0;
        }

        if(pos > 39) {
            fprintf(stderr,"internal error: (ikmod writetiming) pos > 39\n");
        }
        
        if (verbose > 1)  {
            printf("writing evt frames: %4d offset:%4d pos: %2d len: %d  text:", frames, e->textoffs, pos, len);
            writewithcr(stdout, linebuf);
            printf("\n");
        }
        lastframes = e->frames;

        if(ikformat == 0) {
            put16(f, 0x100 + frames);
            fputc(pos,f);
            fputc(len - 1, f);
        } else {
            fprintf(f, "    !byte <$%04x, $01 + >$%04x, %2d, %2d ; frames, pos, len-1  text:", frames, frames, pos, len - 1);
            writewithcr(f, linebuf);
            fprintf(f, "\n");
        }
        
        
        if (karaoke_text[n->textoffs - 1] == '\n') {
            textptr = n->textoffs;
            pos = 0;
            len = 0;
        }

        e++; n++;
        //pos++;
        pos+=len;
        num++;
    }
    if (verbose) printf("%d events written to output file\n", num);
    
    frames = 50; // FIXME
    if(ikformat == 0) {
        put16(f, 0x100 + frames);
        fputc(0 ,f);
        fputc(0xff ,f);
    } else {
        fprintf(f, "    !byte <$%04x, $01 + >$%04x, 0, $ff ; frames, end of list\n", frames, frames);
    }
    num++;

    return num * 4;
}

static int writepadding(FILE *f)
{
    unsigned int i, pad, pos;
    // pad to next 0x1000
    pos = ftell(f);
    pad = pos + 0xfff;
    pad &= ~0xfff;
    pad -= pos;
    for (i = 0; i < pad; i++) {
        fputc(0xff ,f);
    }
    return 0;
}

static int writeik(char *name)
{
    FILE *f = fopen(name, "wb");
    if (f == NULL) {
        return -1;
    }

    text_length = strlen(karaoke_text) + 2;

    songname_offset = 0x1c;
    author_offset = songname_offset + strlen(karaoke_songname) + 1;
    data_offset = author_offset + strlen(karaoke_author) + 1;
    text_addr = sid_data_addr + sid_data_length;
    timing_addr = text_addr + text_length;

    if (verbose) {
        printf(
            "writeikmod: %s\n"
            " songname offset: %04x '%s'\n"
            " author offset:   %04x '%s'\n"
            " data offset:     %04x\n"
            " text addr:       %04x len: %04x\n"
            " timing addr:     %04x len: %04x\n",
            name, songname_offset, karaoke_songname, author_offset, karaoke_author,
            data_offset,
            text_addr, text_length,
            timing_addr, timing_length
              );
    }

    if (ikformat == 0) {
        fputc(0xc9, f);
        fputc(0xcb, f);
        fputc(0xab, f);

        put16(f, songname_offset);
        put16(f, author_offset);

        put16(f, sid_init_addr);
        put16(f, sid_play_addr);
        fputc(music_speed, f);

        put16(f, data_offset);
        put16(f, data_length);      // 0x0e
        put16(f, sid_data_addr);

        put16(f, text_addr);
        put16(f, timing_addr);

        fputc(start_counter, f);
        put16(f, volume_addr);
        fputc(fade_speed, f);
        put16(f, fade_delay);
        
        writename(f);
        writeauthor(f);

        writesiddata(f);
        writetext(f);
        timing_length = writetiming(f);
 
        writepadding(f);
        
        /* some fields in the header are unknown until the data portion was created,
           so fill them out here */
        data_length = sid_data_length + text_length + timing_length;
        fseek(f, 0x0e, SEEK_SET);
        put16(f, data_length);      // 0x0e

    } else {
        fprintf(f, "datablockaddr=$%04x\n", sid_data_addr);
        fprintf(f, "    !initmem $ff\n");
        fprintf(f, "\n;------------------------------------------------------------------------------\n");
        fprintf(f, "    * = $0000\n");
        fprintf(f, "start:\n");
        fprintf(f, "    !byte $c9, $cb, $ab             ; +00  IK+\n");
        fprintf(f, "    !word songname - start          ; +03\n");
        fprintf(f, "    !word authorname - start        ; +05\n\n");
        fprintf(f, "    !word $%04x                     ; +07  init\n", sid_init_addr);
        fprintf(f, "    !word $%04x                     ; +09  play\n", sid_play_addr);
        fprintf(f, "    !byte $%02x                       ; +0b  speed\n\n", music_speed);
        fprintf(f, "    !word datablock - start         ; +0c\n");
        fprintf(f, "    !word datablockend - datablock  ; +0e\n");
        fprintf(f, "    !word datablockaddr             ; +10\n\n");
        fprintf(f, "    !word textaddr                  ; +12\n");
        fprintf(f, "    !word timingaddr                ; +14\n");
        fprintf(f, "    !byte $%02x                       ; +16\n", start_counter);
        fprintf(f, "    !word $%04x                     ; +17\n", volume_addr);
        fprintf(f, "    !byte $%02x                       ; +19\n", fade_speed);
        fprintf(f, "    !word $%04x                     ; +1a\n", fade_delay);
        fprintf(f, "\n;------------------------------------------------------------------------------\n");
        fprintf(f, "songname:\n");
        fprintf(f, "           ;0123456789012345678901234567890 <- max 31 characters\n");
        writescreencode(f, karaoke_songname);
        fprintf(f, "authorname:\n");
        fprintf(f, "           ;0123456789012345678901234567890 <- max 31 characters\n");
        writescreencode(f, karaoke_author);
        fprintf(f, ";------------------------------------------------------------------------------\n");
        fprintf(f, "datablock:\n");
        fprintf(f, "!pseudopc datablockaddr {\n");
        fprintf(f, "\n    !binary \"%s\",,$7c+2 ; PSID v2\n", sid_filename); // psid v2
//        fprintf(f, "\n    !binary \"%s\",,$76+2 ; PSID v1\n", sid_filename); // psid v1
        fprintf(f, "\ntextaddr:\n");
        writescreencode(f, karaoke_text);
        fprintf(f, "    !byte $ff\n");
        fprintf(f, "\ntimingaddr:\n");
        writetiming(f);
        fprintf(f, "}\n");
        fprintf(f, "datablockend:\n");
        fprintf(f, ";------------------------------------------------------------------------------\n\n");
        fprintf(f, "    !fill $1000 - (* & $fff), $ff ; fill up to next 4k block\n");
//        fprintf(f, "    !align $fff, 0  ; fill up to next 4k block\n");
    }

    if (verbose) {
        printf(
            " %d of %d events were skipped.\n",
            events_skipped, karoke_events
              );
    }
    
    fclose(f);

    return 0;

}

int writeikmod(char *name)
{
    ikformat = 0;
    return writeik(name);
}

int writeikasm(char *name)
{
    ikformat = 1;
    return writeik(name);
}
