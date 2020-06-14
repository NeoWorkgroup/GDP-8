%{
/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#define COLOR		"\033[1;44;33m"
#define ENDCOLOR	"\033[0m"
%}

WORD		[a-zA-Z][0-9a-zA-Z]*
VALUE		[0-9a-fA-F]+
ADDR		"$"({VALUE}|{WORD})
INTERMEDIATE	"#"{VALUE}
REGISTER	"%"({VALUE}|{WORD})
INDIRECT	"@"{WORD}
PSEUDOOP	^"."{WORD}
LABEL		{WORD}":"
COMMENT		";".*\n
QUOTE		\".*\"
BLANKSPACE	[ \t]+

%%

{LABEL}		printf("\n" COLOR "LABEL:" ENDCOLOR "\t%s\n", yytext);
{ADDR}		printf(COLOR "ADDR:" ENDCOLOR "\t%s\t", yytext);
{INTERMEDIATE}	printf(COLOR "INTERMEDIATE:" ENDCOLOR "\t%s\t", yytext);
{REGISTER}	printf(COLOR "REGISTER:" ENDCOLOR "\t%s\t", yytext);
{WORD}		printf(COLOR "WORD:" ENDCOLOR "\t%s\t", yytext);
{INDIRECT}	printf(COLOR "INDIRECT:" ENDCOLOR "\t%s\t", yytext);
{PSEUDOOP}	printf(COLOR "PSEUDOOP:" ENDCOLOR "\t%s\t", yytext);
{COMMENT}	printf("\n" COLOR "COMMENT:" ENDCOLOR "\t%s", yytext);
{QUOTE}		printf(COLOR "QUOTE:" ENDCOLOR "\t%s\t", yytext);
","		printf(COLOR ",\t" ENDCOLOR);
\n		putchar('\n');
{BLANKSPACE}	;
.		printf("NOPE:\t%s\t", yytext);

%%

int main(void)
{
	yylex();
}
