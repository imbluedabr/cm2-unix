#include <stdio.h>
#include <stdlib.h>

typedef enum { PRINT,INPUT,VAR,IF,GOTO,GOSUB,RET,REM } BasicCommands;
static const char* cmds[] = {"PRINT","INPUT","VAR","IF","GOTO","GOSUB","RET","END"};

static char line_buffer[100][64];

int isspc(char c) { return c == ' ' || c == '\n' || c == '\t'; }
void scpy(char* d, const char* s) { int i=0;for(;s[i];i++) d[i]=s[i]; d[i]=0; }

char* _CURTOK = NULL;
// variation of strtok that can detect strings
char* sstrtok(char* s) {
	int a=0;
	if  (s != NULL) _CURTOK = s;
	if  (!(*_CURTOK)) return NULL;
	for (;*_CURTOK && isspc(*_CURTOK);++_CURTOK) s = _CURTOK;
	for (;*_CURTOK && (a||!isspc(*_CURTOK));++_CURTOK)
		if (*_CURTOK=='"') { if(!a) a=1; else break; }
	*(_CURTOK++) = 0;
	return s;
}

int getcmd(const char* s) {
	printf("y: %s\n", cmds[0]);
	for (int i = 0; i < 4; ++i) {
		printf("%s\n", cmds[i]);
		if (strncmp(s, cmds[i], 5) == 0) {
			return i;
		};
	}
	return -1;
}

void berror(int linenum, const char* e) {
	if (linenum == -1)
		printf("ERROR: %s\n", e);
	else
		printf("ERROR AT %x: %s\n", linenum, e);
	exit(1);
}

typedef int (*BasicCmd)(int,char*);
int cprint(int ln, char* s) {
	/*char* token = sstrtok(s);
	for (;token && *token;token = sstrtok(NULL))
	{
		if (token[0] == '"') {
			for(++token;*token&&*token!='"';++token) putc(*token);
		}
		else printf("%x", emath(token));
	}
	putc('\n');*/
	puts("yeet\n");
	return ln;
}/*
int cinput(int ln, char* s) {
	char vs[16], vn[16]; scpy(vn, strtok(s));
	if (!vn) berror(ln, "INVALID ARGS");
	printf("%s? ", vn); fgets(vs, 15, stdin);
	setvar(vn, atoi(vs));
	return ln;
}
int cvar(int ln, char *s) {
	char *tok = strtok(s);
	if (!tok) berror(ln, "INVALID ARGS");
	char vn[16]; scpy(vn, tok);
	tok = strtok(NULL);
	if (!tok) berror(ln, "INVALID ARGS");
	setvar(vn, emath(tok));
	return ln;
}
int runcmd(int,char*);
int cif(int ln, char* s) {
	char *tok = strtok(s);
	if (!tok || !*_CURTOK) berror(ln, "INVALID IF STATEMENT");
	return emath(tok) ? runcmd(ln, _CURTOK) : ln;
}
int cgoto(int ln, char* s) {
	char *tok = strtok(s); if (!tok) berror(ln, "INVALID GOTO");
	return emath(tok)-1;
}
int cgosub(int ln, char* s) {
	char *tok = strtok(s); if (!tok) berror(ln, "INVALID GOSUB");
	int c=emath(tok); lnpush(ln); return c-1;
}
int cret(int ln, char* s) { return lnpop(); }
int crem(int ln, char* s) { exit(0); }
*/
BasicCmd bfuncs[] = {cprint};

char buf[64];
int runcmd(int ln, char* s) {
	strlcpy(buf, s, 63);
	
	for (int i = 0; i < 63; i++) {
		if (buf[i] == ' ') {
			buf[i] = '\0';
			break;
		}
	}
	printf("cmd: %s\n", buf);
	int cmd = getcmd(buf);
	return 0;
}

int main(const char** argv)
{
	runcmd(0, "PRINT \"yeet\"");

	int line_count = 0;
	
	while(fgets(line_buffer[line_count++], 63, stdin));

	for (int i = 0; i < line_count; i++) {
		puts(line_buffer[i]);
	}

	return 0;
}



