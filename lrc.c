
static unsigned int extradelay = 250;

int readlrc(char *name)
{
    FILE *f = fopen(name, "rb");
    unsigned int c, istag, tagptr, lineptr, textptr;
    unsigned int minsec, min, sec, hsec, events, lastoffset;
    char tagbuf[0x100];
    char tag[0x100];
    char tagarg[0x100];
    char linebuf[0x100];
    int textstart,firstline;

    if (f == NULL) {
        return -1;
    }

    istag = 0;
    lineptr = 0;
    textptr = 0;
    events = 0;
    lastoffset = 0;
    textstart = 0;
    firstline=1;
    while (!feof(f)) {
        c = fgetc(f);
        if (istag == 0) {
            if (c == '<') {
                istag = 1;
                tagptr = 0;
            } else if (c == '[') {
                istag = 2;
                tagptr = 0;
            } else {
                if (c == '\n') {
                    linebuf[lineptr] = 0;
                    lineptr = 0;
                    if (verbose) printf("line:%s\n", linebuf);
                    if(textstart) {
                        if(!firstline) {
                            strcat(&karaoke_text[textptr], "\n");
                            textptr++;
                        }
                        strcat(&karaoke_text[textptr], linebuf);
                        textptr+=strlen(linebuf);
                        firstline=0;
                    }
                } else {
                    //printf("start:'%c'%02x\n",c,c);
                    if (!isblank(c)) {
                        textstart = 1;
                    }
                    if(textstart) {
                        linebuf[lineptr] = c;
                        lineptr++;
                    }
                }
            }
        } else if (istag == 1) {
            if (c == '>') {
                istag = 0;
                tagbuf[tagptr] = 0;
                if (verbose) printf("tag:<%s>\n", tagbuf);
                min = sec = hsec = 0;
                sscanf(tagbuf, "%d:%d.%d", &min, &sec, &hsec);
                minsec = ((((min * 60) + sec) * 100) + hsec) * 10;
                if (verbose) printf("minsec:%d %d %d %d\n", minsec, min, sec, hsec);
                karaoke_timing[events].format = KT_FORMAT_FRAMES;
                karaoke_timing[events].textoffs = textptr + lineptr;
                //karaoke_timing[events].textoffs = lastoffset;
                karaoke_timing[events].frames = minsec / 20;
                events++;
                //lastoffset = textptr + lineptr;
            } else {
                tagbuf[tagptr] = c;
                tagptr++;
            }
        } else if (istag == 2) {
            if (c == ']') {
                istag = 0;
                tagbuf[tagptr] = 0;
                if (verbose) printf("tag:[%s]\n", tagbuf);
                if ((tagbuf[0] >= '0') && (tagbuf[0] <= '9')) {
                    min = sec = hsec = 0;
                    sscanf(tagbuf, "%d:%d.%d", &min, &sec, &hsec);
                    minsec = ((((min * 60) + sec) * 100) + hsec) * 10;
                    if (verbose) printf("minsec:%d %d %d %d\n", minsec, min, sec, hsec);
#if 1
                    karaoke_timing[events].format = KT_FORMAT_FRAMES;
                    karaoke_timing[events].textoffs = textptr + lineptr;
                    //karaoke_timing[events].textoffs = lastoffset;
                    karaoke_timing[events].frames = minsec / 20;
                    events++;
#endif
                    //lastoffset = textptr + lineptr;
                } else {
                    sscanf(tagbuf, "%s %[^\n]\n", tag, tagarg);
                    printf("tag: %s tagarg: %s\n", tag, tagarg);
                    if(!strcmp(tag,"ti:")) {
                        strcpy(karaoke_songname, tagarg);
                    } else if(!strcmp(tag,"ar:")) {
                        strcpy(karaoke_author, tagarg);
                    //} else if(!strcmp(tag,"al:")) {
                    } else if(!strcmp(tag,"offset:")) {
                        extradelay = atoi(tagarg);
                    //} else if(!strcmp(tag,"re:")) {
                    //} else if(!strcmp(tag,"ve:")) {
                    }
                }
            } else {
                tagbuf[tagptr] = c;
                tagptr++;
            }
        }
    }

    karaoke_timing[events].format = KT_FORMAT_END;
    karaoke_timing[events].textoffs = 0;
    karaoke_timing[events].frames = 0;

    fclose(f);

//    printf("text:'%s'\n", karaoke_text);
#if 0
    events = 0;
    while(karaoke_timing[events].format != KT_FORMAT_END) {
        if (karaoke_text[karaoke_timing[events].textoffs] == '\n') {
            while (karaoke_text[karaoke_timing[events].textoffs] != ' ') {
                karaoke_timing[events].textoffs--;
            }
            karaoke_timing[events].textoffs++;
            // move next event before CR
            events++;
            while (karaoke_text[karaoke_timing[events].textoffs] != '\n') {
                karaoke_timing[events].textoffs--;
            }
        }
        events++;
    }
#endif
    return 0;
}

//------------------------------------------------------------------------------

int writelrc(char *name)
{
    unsigned int textptr = 0;
    unsigned int linepos = 0;
    unsigned int events = 0;
    int c;

    if (verbose) printf("writelrc:%s\n", name);
    FILE *f = fopen(name, "wb");
    if (f == NULL) {
        return -1;
    }

    fprintf(f, "[ti: %s]\n", karaoke_songname);
    fprintf(f, "[ar: %s]\n", karaoke_author);
//    fprintf(f, "[al: %s]\n", );
    fprintf(f, "[offset: %d]\n", extradelay);
    fprintf(f, "[re: ikconv]\n");
    fprintf(f, "[ve: 0.1]\n"); // FIXME

    while ((c = karaoke_text[textptr]) != 0) {
//        printf("%d textptr <%04x>\n", textptr, events );
        while ((textptr >= karaoke_timing[events].textoffs) && (karaoke_timing[events].format != 0xff)) {
            if (linepos > 0) {
                fprintf(f, "<%04x>", karaoke_timing[events].textoffs);
            } else {
                fprintf(f, "[%04x]", karaoke_timing[events].textoffs);
            }
            events++;
        }
        if (c == '\n') {
            linepos = 0;
            //textptr++;
        } else {
            linepos++;
        }
        fputc(c, f);
        textptr++;
    }

    fclose(f);

    return 0;
}
