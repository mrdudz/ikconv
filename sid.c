 
unsigned char sid_memory[0x10000];

unsigned int sid_data_addr;
unsigned int sid_data_length;

unsigned int sid_init_addr;
unsigned int sid_play_addr;

char sid_filename[0x100];
char sid_name[0x40];
char sid_author[0x40];
char sid_copyright[0x40];

static unsigned int sid_get16(FILE *f)
{
    unsigned int n = fgetc(f) << 8;
    n |= fgetc(f);
    return n;
}

static int sid_put16(FILE *f, int val)
{
    fputc((val >> 8) & 0xff, f);
    return fputc(val & 0xff, f);
}

int readsid(char *name)
{
    unsigned char c;
    int version, offset, load;
    if (verbose) printf("readsid:%s\n", name);
    FILE *f = fopen(name, "rb");
    if (f == NULL) {
        return -1;
    }
    strcpy(sid_filename, name);

    c = fgetc(f);if (c != 'P') return -1;
    c = fgetc(f);if (c != 'S') return -1;
    c = fgetc(f);if (c != 'I') return -1;
    c = fgetc(f);if (c != 'D') return -1;

    version = sid_get16(f);
    offset = sid_get16(f);
    load = sid_get16(f);

    sid_init_addr = sid_get16(f);
    sid_play_addr = sid_get16(f);

    fseek(f, 0x16, SEEK_SET);
    memset(sid_name, 0, 0x21);
    memset(sid_author, 0, 0x21);
    memset(sid_copyright, 0, 0x21);
    fread(sid_name, 1, 0x20, f);
    fread(sid_author, 1, 0x20, f);
    fread(sid_copyright, 1, 0x20, f);

    strcpy(karaoke_songname, sid_name);
    strcpy(karaoke_author, sid_author);

    fseek(f, offset, SEEK_SET);
    sid_data_addr = get16(f);
    sid_data_length = fread(&sid_memory[sid_data_addr], 1, 0xffff, f);

    if (verbose) {
        printf("PSID version: %d data offset: %02x data load: %04x\n", version, offset, load);
        printf("sid init: %04x play: %04x addr: %04x len: %04x\n", sid_init_addr, sid_play_addr, sid_data_addr, sid_data_length);
    }

    fclose(f);
    return 0;
}

int writesid(char *name)
{
    if (verbose) printf("writesid:%s\n", name);
    FILE *f = fopen(name, "wb");
    if (f == NULL) {
        return -1;
    }
    strcpy(sid_filename, name);
    fputc('P', f);
    fputc('S', f);
    fputc('I', f);
    fputc('D', f);
    sid_put16(f, 2); // version
    sid_put16(f, 0x7c); // data offset
    sid_put16(f, 0); // load address (take from binary)
    sid_put16(f, sid_init_addr); // init
    sid_put16(f, sid_play_addr); // play
    sid_put16(f, 1); // songs
    sid_put16(f, 1); // default song
    sid_put16(f, 0); // speeds
    sid_put16(f, 0); // speeds

    memset(sid_name, 0, 0x21);
    memset(sid_author, 0, 0x21);
    strcpy(sid_name, karaoke_songname);
    strcpy(sid_author, karaoke_author);

    //+16 name
    fseek(f, 0x16, SEEK_SET);
    fwrite(sid_name, 1, 0x20, f);
    //+36 author
    fwrite(sid_author, 1, 0x20, f);
    //+56 released
    fwrite(sid_copyright, 1, 0x20, f);
    //+76 binary data
    fseek(f, 0x7c, SEEK_SET);
    put16(f, sid_data_addr);
    fwrite(&sid_memory[sid_data_addr], 1, sid_data_length, f);
    fclose(f);
    return 0;
}
