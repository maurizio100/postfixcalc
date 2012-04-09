/**
   @file calc.c
   @author Maurizio Rinder 0828852
   @date 06.04.2012
**/
/******************************************
This is a program that simulates a postfix calculator
Postfix notations work like that:
2 * ( 1 + 3) => 2 1 3 + *
1 + 2 => 1 2 +
and so on...
The calculator implements arithmetic Operations (+,-,*,/) and
cos, sin (s and c).
For saving the digits, a stack is used.

the synopsis is:
calc [-i] [-a] [file1[file2 ...]]
-a ... absolute value of result
-i ... integer value of result
*******************************************/
/*****************************************
Includes and Definitions
*****************************************/
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#define BFSIZE 1025
#define ERRLEN 1025
#define TRUE 1
/*************************************
Global variables
*************************************/
static char *prgname;
static char *errmsg;
static FILE *src_file;
/****************************************
Stack definition and function
****************************************/
typedef struct{
  unsigned int maxsize;
  unsigned int actsize;
  double *esp;
}Stack;

static Stack *stack;
/****************************************
Function prototypes
****************************************/
static int create_stack(const unsigned int size);
static void push(const double value);
static double pop();
static void destroy_stack();
static double calculate(const char* calculation);
/****************************************
Error routines
****************************************/
static void usage(void);
static void file_error(char* errmsg, const char* file);
static void stack_error(char* errmsg);
static void calc_error(char* errmsg);
/**************************************
Main Procedure
**************************************/
int main(int argc, char **argv){
  /*****Variables*****/
  int c = 0;
  /*
    a flagarray which determines how
    calculated results are written to stdout
    outmode[0] ... absolute output
    outmode[1] ... integer output
  */
  int outmode[] = {0, 0};

  double result = 0.0;
 
  int fileread = 0;   
  char *in_line = 0;  
  char *res = 0;
  /*****Usage test****/
  if(argc > 0) prgname = argv[0];

  while(( c = getopt( argc,argv,"ai") ) != EOF ){
    switch( c ){
    case 'a':
      outmode[0] = TRUE; break;
    case 'i':
      outmode[1] = TRUE; break;
    case '?': usage(); break;
    default: assert(0);
    }
  } 

  /******Processing inputs*******/
  if( create_stack(BFSIZE) == 0 ){ 
    errmsg = (char*)malloc(ERRLEN * sizeof(char));
    errmsg = "Could not create Stack. Size has to be > 0."; 
    stack_error(errmsg);
  }

  if(optind < argc){  
    src_file = fopen(argv[optind], "r"); 

    if( src_file == 0 ){
      errmsg = (char*)malloc(ERRLEN * sizeof(char));
      errmsg = "No such file.";
      file_error(errmsg, argv[optind]); 
    }

    optind++;
    fileread = TRUE; 
  } else { src_file = stdin; }

  in_line = (char*)malloc(BFSIZE * sizeof(char));
  while( (res=fgets(in_line, BFSIZE, src_file))!=NULL || (optind < argc) ){
   
    /*for fileinput only - loads the next file from arguments*/ 
    if( res == NULL && fileread ){
      fclose(src_file);      
      src_file = fopen(argv[optind], "r");
     
      if( src_file == 0 ){
	free(in_line);
	errmsg = "No such file.";
	file_error(errmsg, argv[optind]);   
      }
      
      optind++;
      continue;
    }
    /*calculation of line read*/
    result = calculate(in_line);
	if( outmode[0] == TRUE ) result = abs(result);
	if( outmode[1] == TRUE ) printf("%d\n",(int)result);
	else printf("%f\n",result);
  }

  if( fileread ) fclose(src_file);

  /**********clearing and exit***************/
  destroy_stack();
  free(in_line);
  exit(EXIT_SUCCESS);
}
/************************************
Function definitions
 ***********************************/
static double calculate(const char* calculation){
  double operand = 0.0;
  char *endptr = (char*)malloc(BFSIZE * sizeof(char));
  strcpy(endptr, calculation);
  
   while( (*endptr != '\n') && (*endptr != EOF)){ 
    operand = strtod(endptr, &endptr);

    if( operand == 0){
      if( *endptr == ' ' ) endptr++;

      switch(*endptr){
      case '+': push(pop() + pop());break;
      case '-': 
	operand = pop();
	push((pop() - operand)); break;
      case '*': push(pop() * pop()); break;
      case '/': 
	operand = pop();
	push(pop() / operand); break;
      case 's': push(sin(pop())); break;
      case 'c': push(cos(pop())); break;
      default: 
	fclose(src_file);
	errmsg="Not a valid operation."; 
	calc_error(errmsg);
      }

      endptr++;
    }else{
      push(operand);
    }
   }

   if( stack->actsize > 1 ){
     fclose(src_file);
     errmsg = (char*)malloc(ERRLEN * sizeof(char));
     errmsg = "Wrong use of Postfix notation.";
     calc_error(errmsg);
   }

   return pop();
}

static int create_stack(const unsigned int size){
  if( size <= 0 ) return 0;

  stack = (Stack*)malloc(sizeof( Stack ));
  stack->maxsize = size;
  stack->actsize =0;
  stack->esp=(double*)malloc(sizeof(double)*size);

  return 1;
}
static void push(const double value){
  /*Error message when stack is full*/
  stack->esp[stack->actsize] = value;
  stack->actsize++;
 
}
static double pop(){
  /*Error message when stack is empty*/
  stack->actsize--;
  return stack->esp[stack->actsize]; 
}
static void destroy_stack(){
  free(stack->esp);
  free(stack);
  stack = 0;
}
/************************************
Error routines definitions
 ***********************************/
static void usage(void){
  (void)fprintf(stderr,"Usage: %s [-a] [-i] [file1[file2 ...]]\n",prgname);
  exit(EXIT_FAILURE);
}
static void file_error(char* errmsg, const char* file){
  (void)fprintf(stderr,"Error: Problem with file %s. %s\n", file, errmsg);
  free(errmsg);
  exit(EXIT_FAILURE);
}
static void stack_error(char* errmsg){
  (void)fprintf(stderr,"Error: Problem with stack. %s\n", errmsg);
  free(errmsg);
  if( stack != NULL) destroy_stack(); 
  exit(EXIT_FAILURE);
}
static void calc_error(char* errmsg){
  (void)fprintf(stderr,"Error: Problem with calculation. %s\n", errmsg);
  free(errmsg);
  if( stack != NULL ) destroy_stack();
  exit(EXIT_FAILURE);
}

