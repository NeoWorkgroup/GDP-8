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
%}

WORD		[a-zA-Z][0-9a-zA-Z]*
VALUE		[0-9a-fA-F]+
ADDR		"$"({VALUE}|{WORD})
INTERMEDIATE	"#"{VALUE}
REGISTER	"%"({VALUE}|{WORD})
INDIRECT	"@"{WORD}
PSEUDOOP	^"."{WORD}
COMMENT		";".*\n
QUOTE		\".*\"
BLANKSPACE	[ \t]+

%%

{WORD}":"	printf("\nLABEL:\t%s\n", yytext);
{ADDR}		printf("ADDR:\t%s\t", yytext);
{INTERMEDIATE}	printf("INTERMEDIATE:\t%s\t", yytext);
{REGISTER}	printf("REGISTER:\t%s\t", yytext);
{WORD}		printf("WORD:\t%s\t", yytext);
{INDIRECT}	printf("INDIRECT:\t%s\t", yytext);
{PSEUDOOP}	printf("PSEUDOOP:\t%s\t", yytext);
{COMMENT}	printf("\nCOMMENT:\t%s", yytext);
{QUOTE}		printf("QUOTE:\t%s\t", yytext);
","		printf(",\t");
\n		putchar('\n');
{BLANKSPACE}	;
.		printf("NOPE:\t%s\t", yytext);

%%

int main(void)
{
	yylex();
}
