#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*constants for true and false*/
#define FALSE 0
#define TRUE 1
#define getName(var, str) sprintf(str, "%s", #var)

/*enumerated types for token types*/
typedef enum{
	ID, INTLITERAL, READ, WRITE,
	PLUSOP, MINUSOP, ASSIGNOP, LPAREN, RPAREN,
	COMMA, SEMICOLON, SCANEOF, IF, ELSE, WHILE,
	DIVOP, MULTOP, BOOLOP, LBRACK, RBRACK, MAIN
} token;

/*functions declarations related to scanner*/
token scanner();
void clear_buffer();
void buffer_char(char c);
token check_reserved();
void lexical_error();

/*functions declarations related to parser*/
void parser();
void program();
void statement_list();
void statement();
void id_list();
void expression_list();
void expression();
void boolean_expression();
void bool_op();
void term();
void factor();
void add_op();
void mult_op();
void match(token tok);
void syntax_error();

/*global variables*/
FILE *fin;				/*source file*/
FILE *fout;				/*output file*/
token next_token;		/*next token in source file*/
char token_buffer[100]; /*token buffer*/
int token_ptr;			/*buffer pointer*/
int line_num = 1;		/*line number in source file*/
int error = FALSE;		/*flag to indicate error*/
int file_ended = FALSE;

/****************************************************************/

/*returns next token from source file*/
token scanner(){
	char c;						/*current character in source file*/
	clear_buffer();				/*empty token buffer*/
	while(TRUE){				/*loop reads and returns next token*/
		c = getc(fin);
		
		if(c == EOF)
			return SCANEOF;
		
		else if(isspace(c)){
			if(c == '\n')
				line_num = line_num + 1;
		}

		else if(isalpha(c) && c != '*' && c != '/' && c != '{' && c != '}'){
			buffer_char (c);
			c = getc(fin);
			while(isalnum(c) || c == '_'){
				buffer_char(c);
				c = getc(fin);
			}
			ungetc(c, fin);
			return check_reserved();
		}

		/*integer literal*/
		else if(isdigit(c)){
			buffer_char(c);
			c = getc(fin);
			while(isdigit(c)){
				buffer_char(c);
				c = getc(fin);
			}
			ungetc(c, fin);
			return INTLITERAL;
		}

		else if (c == '(')
			return LPAREN;

		else if (c == ')')
			return RPAREN;

		else if (c == ',')
			return COMMA;

		else if (c == ';')
			return SEMICOLON;

		else if (c == '+')
			return PLUSOP;

		else if (c == '-')
			return MINUSOP;

		else if (c == '*')
			return MULTOP;

		else if (c == '{')
			return LBRACK;

		else if (c == '}')
			return RBRACK;

		else if (c == '/'){
			c = getc(fin);

			/*checks if a comment or division operater*/
			if(c == '/'){
				do
					c = getc(fin);
				while (c != '\n');
				line_num = line_num + 1;
			} else {
				ungetc(c, fin);
				return DIVOP;
			}
		}

		else if (c == '='){
			c = getc(fin);
			if (c == '='){
				return BOOLOP;
			} else {
				ungetc(c, fin);
				lexical_error();
			}
		}

		else if (c == '!'){
			c = getc(fin);
			if (c == '='){
				return BOOLOP;
			} else {
				ungetc(c, fin);
				lexical_error();
			}
		}

		else if (c == '<'){
			c = getc(fin);
			
			/*Checks to see if <= or just < */
			if (c == '='){
				return BOOLOP;
			} else {
				ungetc(c, fin);
				return BOOLOP;
			}
		}

		else if (c == '>'){
			c = getc(fin);

			/*Checks to see if >= or just > */
			if (c == '='){
				return BOOLOP;
			} else {
				ungetc(c, fin);
				return BOOLOP;
			}
		}

		else if (c == ':'){
			c = getc(fin);
			if(c == '='){
				return ASSIGNOP;
			} else {
				ungetc(c, fin);
				lexical_error();
			}
		}

		else{
			lexical_error();
		}

	}
}

/*************************************************************/

/*clears the buffer*/
void clear_buffer(){
	token_ptr = 0;
	token_buffer[token_ptr] = '\0';
}

/***********************************************************/

/*appends the character to buffer*/
void buffer_char(char c){
	token_buffer[token_ptr] = c;
	token_ptr = token_ptr + 1;
	token_buffer[token_ptr] = '\0';
}

/*****************************************************************/

/*checks whether buffer is reserved word or identifier*/
token check_reserved(){
	if (strcmp(token_buffer, "read") == 0)
		return READ;
	else if (strcmp(token_buffer, "write") == 0)
		return WRITE;
	else if (strcmp(token_buffer, "if") == 0)
		return IF;
	else if (strcmp(token_buffer, "else") == 0)
		return ELSE;
	else if (strcmp(token_buffer, "while") == 0)
		return WHILE;
	else if (strcmp(token_buffer, "main") == 0)
		return MAIN;
	else
		return ID;
}

/********************************************************************/

/*reports lexical error and sets error flag*/
void lexical_error(){
	printf("Lexical error on line %d\n", line_num);
	error = TRUE;
}

/*************************************************************************/

/*parses source file*/
void parser(){
	next_token = scanner();
	program();
	match(SCANEOF);
}

/***********************************************************************/

/*parses a program*/
/* <program> --------> begin<stmtlist>end */
void program(){
	match(MAIN);
	match(LBRACK);
	statement_list();
	match(RBRACK);
}

/************************************************************************/

/*parses list of statements*/
/* <stmtlist> --> <stmt>{<stmt>} */
void statement_list(){
	statement();
	while (TRUE){
		if(next_token == ID || next_token == READ || next_token == WRITE || next_token == WHILE || next_token == IF)
			statement();
		else
			break;
	}
}

/**************************************************************************/

/*parses one statement*/
/* <stmt> --> id:=<expr>;
   <stmt> --> read(<idlist>);
   <stmt> --> write(<exprlist>);
   <stmt> --> while(<boolexpr>){<stmtlist>}
   <stmt> --> if(<boolexpr>){<stmtlist>}[else{<stmtlist>}] */
void statement(){
	if(next_token == ID){
		match(ID);
		match(ASSIGNOP);
		expression();
		match(SEMICOLON);
	} else if(next_token == READ){
		match(READ);
		match(LPAREN);
		id_list();
		match(RPAREN);
		match(SEMICOLON);
	} else if(next_token == WRITE){
		match(WRITE);
		match(LPAREN);
		expression_list();
		match(RPAREN);
		match(SEMICOLON);
	} else if(next_token == IF){
		match(IF);
		match(LPAREN);
		boolean_expression();
		match(RPAREN);
		match(LBRACK);
		statement_list();
		match(RBRACK);
		if (next_token == ELSE){
			match(ELSE);
			match(LBRACK);
			statement_list();
			match(RBRACK);			
		}
	} else if(next_token == WHILE){
		match(WHILE);
		match(LPAREN);
		boolean_expression();
		match(RPAREN);
		match(LBRACK);
		statement_list();
		match(RBRACK);
	} else
		syntax_error();
}

/*************************************************************************/

/*parses list of identifiers*/
/* <idlist> --> id{,id} */
void id_list(){
	match(ID);
	while(next_token == COMMA){
		match(COMMA);
		match(ID);
	}
}

/************************************************************************/

/*parses a boolean expression*/
/* <boolexpr> --> id|integer|<expr> <boolop> id|integer|<expr> */
void boolean_expression(){
	if (next_token == ID)
		match(ID);
	else if (next_token == INTLITERAL)
		match(INTLITERAL);
	else 
		expression();
	bool_op();
	if (next_token == ID)
		match(ID);
	else if (next_token == INTLITERAL)
		match(INTLITERAL);
	else
		expression();
}

/**************************************************************************/

/*parses a boolean operator */
/* <boolop> --> < | > | <= | >= | == | != */
void bool_op(){
	if(next_token == BOOLOP)
		match(next_token);
	else
		syntax_error();	
}

/************************************************************************/

/*parses list of expressions*/
/* <explist> --> <exp>{,<exp>} */
void expression_list(){
	expression();
	while(next_token == COMMA){
		match(COMMA);
		expression();
	}
}

/**********************************************************************/

/*parses one expression*/
/* <exp> --> <term>{<adop><term>} */
void expression(){
	term();
	while(next_token == PLUSOP || next_token == MINUSOP){
		add_op();
		term();
	}
}

/********************************************************************/

/*parses one term*/
/* <term> --> <factor>{<multop><factor>} */
void term(){
	factor();
	while(next_token == MULTOP || next_token == DIVOP){
		mult_op();
		factor();
	}
}
/*********************************************************************/

/*parses one term*/
/* <factor> --> id
   <factor> --> integer
   <factor> --> (<expr>) */
void factor(){
	if(next_token == ID)
		match(ID);
	else if(next_token == INTLITERAL)
		match(INTLITERAL);
	else if(next_token == LPAREN){
		match(LPAREN);
		expression();
		match(RPAREN);
	}  else
		syntax_error();
}

/************************************************************************/

/*parses plus or minus operator*/
/* <adop> --> +|- */
void add_op(){
	if(next_token == PLUSOP || next_token == MINUSOP)
		match(next_token);
	else
		syntax_error();
}

/**********************************************************************/

/*parses multiplier or divider operator*/
/* <multop> --> *|/ */
void mult_op(){
	if(next_token == MULTOP || next_token == DIVOP)
		match(next_token);
	else
		syntax_error();
}

/***************************************************************************/

/*checks whether the expected token and the actual token match, and also reads the next token from source file*/
void match(token tok){
	if(tok == next_token)
		;
	else
		syntax_error();

	next_token = scanner();
}

/******************************************************************************/

/*reports syntax error*/
void syntax_error(){
	printf("syntax error on line %d\n", line_num);
	error = TRUE;
	
	/*Informs the user what the syntax error is */
	char the_token[100];
	token checker = next_token;
	if (checker == ID)
		strcpy(the_token, "ID ");
	else if (checker == INTLITERAL)
		strcpy(the_token, "INTLITERAL ");
	else if (checker == READ)
		strcpy(the_token, "READ ");
	else if (checker == WRITE)
		strcpy(the_token, "WRITE ");
	else if (checker == PLUSOP)
		strcpy(the_token, "PLUSOP ");
	else if (checker == MINUSOP)
		strcpy(the_token, "MINUSOP ");
	else if (checker == LPAREN)
		strcpy(the_token, "LPAREN ");	
	else if (checker == RPAREN)
		strcpy(the_token, "RPAREN ");
	else if (checker == ASSIGNOP)
		strcpy(the_token, "ASSIGNOP ");
	else if (checker == SEMICOLON)
		strcpy(the_token, "SEMICOLON ");
	else if (checker == COMMA)
		strcpy(the_token, "COMMA ");
	else if (checker == LBRACK)
		strcpy(the_token, "LBRACK ");
	else if (checker == RBRACK)
		strcpy(the_token, "RBRACK ");
	else if (checker == IF)
		strcpy(the_token, "IF ");
	else if (checker == WHILE)
		strcpy(the_token, "WHILE ");
	else if (checker == MULTOP)
		strcpy(the_token, "MULTOP ");
	else if (checker == DIVOP)
		strcpy(the_token, "DIVOP ");
	else if (checker == BOOLOP)
		strcpy(the_token, "BOOLOP ");
	else if (checker == ELSE)
		strcpy(the_token, "ELSE ");
	else if (checker == MAIN)
		strcpy(the_token, "MAIN ");
	printf("Error was an unexpected %s token \n", the_token);

}

/***********************************************************************************/

int main(){
	int selector;
	char the_infile[100];
	char the_outfile[100];

	/*Prompt user for a choice of what to do */
	printf("Please select fromt the following options:\n");
	printf("1. Scan file and produce tokens.\n");
	printf("2. Parse source code.\n");
	scanf("%d", &selector);
	printf("Please enter the input file name:\n");
	scanf("%s", the_infile);
	fin = fopen(the_infile, "r");
	if (fin == NULL){
		printf("Could not open file %s \n", the_infile);
	}
	if (selector == 1){

		/*Prompt user for output file and check validity*/
		printf("Please enter the output file name:\n");
		scanf("%s", the_outfile);
		fout = fopen(the_outfile, "w");
		if (fout == NULL){
			printf("Could not open file %s \n", the_outfile);
		}

		int printer_line = 1;
		char the_token[100];
		while(TRUE){
			token checker = scanner();
			
			/*checks to see if the line incremented*/
			if (printer_line < line_num){
				fprintf(fout, "\n");
				printer_line = printer_line + 1;
			} 

			/*figures out which token to print*/
			if (checker == ID)
				strcpy(the_token, "ID ");
			else if (checker == INTLITERAL)
				strcpy(the_token, "INTLITERAL ");
			else if (checker == READ)
				strcpy(the_token, "READ ");
			else if (checker == WRITE)
				strcpy(the_token, "WRITE ");
			else if (checker == PLUSOP)
				strcpy(the_token, "PLUSOP ");
			else if (checker == MINUSOP)
				strcpy(the_token, "MINUSOP ");
			else if (checker == LPAREN)
				strcpy(the_token, "LPAREN ");	
			else if (checker == RPAREN)
				strcpy(the_token, "RPAREN ");
			else if (checker == ASSIGNOP)
				strcpy(the_token, "ASSIGNOP ");
			else if (checker == SEMICOLON)
				strcpy(the_token, "SEMICOLON ");
			else if (checker == COMMA)
				strcpy(the_token, "COMMA ");
			else if (checker == LBRACK)
				strcpy(the_token, "LBRACK ");
			else if (checker == RBRACK)
				strcpy(the_token, "RBRACK ");
			else if (checker == IF)
				strcpy(the_token, "IF ");
			else if (checker == WHILE)
				strcpy(the_token, "WHILE ");
			else if (checker == MULTOP)
				strcpy(the_token, "MULTOP ");
			else if (checker == DIVOP)
				strcpy(the_token, "DIVOP ");
			else if (checker == BOOLOP)
				strcpy(the_token, "BOOLOP ");
			else if (checker == ELSE)
				strcpy(the_token, "ELSE ");
			else if (checker == MAIN)
				strcpy(the_token, "MAIN ");
			else if (checker == SCANEOF){
				strcpy(the_token, "SCANEOF ");

				/*end of file is reached, ABORT!*/
				fprintf(fout, "%s", the_token);
				break;
			}

			/*Actually print the token*/
			fprintf(fout, "%s", the_token);
		}
		if (error == FALSE)
			printf("Printing complete. No lexical errors were found. \n");
		else
			printf("Printing complete. \n");
	} else if (selector == 2){

		/*User has selected to parse the code */
		parser();
		if (error == FALSE)
			printf("Parsing was successful. No errors were found.\n");
	} else {
		printf("The selection of what to do with the file is invalid. Please start over. \n");
		main();
	}
	return 0;
}

/*****************************************************************************/