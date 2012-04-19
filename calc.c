/**
   @file calc.c
   @author Maurizio Rinder 0828852
   @date 19.04.2012
**/
/******************************************
This program simulates a postfix calculator
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
Includes and Constants
*****************************************/
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#define BFSIZE (1025)
/****************************************
Stack definition and function
****************************************/
typedef struct{
  unsigned int maxsize;
  unsigned int actsize;
  double *esp;
}Stack;

typedef enum {FALSE, TRUE} bool;
/*************************************
Global variables
*************************************/
/* Name of the program */
static char *prgname;
/* file that is going to read */
static FILE *src_file;
/* stack for the digits read */
static Stack *stack;

/****************************************
Function prototypes
****************************************/
/**
 * @brief Create Stack for calculator
 * @param size The capacity of the stack
 * @return feedback if the stack was created succesfully; FALSE in case of fail
 */
static bool create_stack(const unsigned int size);
/**
 * @brief pushes an item on the stack
 * @param value The value that is going on the stack
 */
static void push(const double value);
/**
 * @brief gets the last item pushed onto the stack
 * @return value last pushed value 
 */
static double pop(void);
/**
 * @brief clears the stack from the storage
 */
static void destroy_stack(void);
/**
 * @brief a check-function that determines if the stack is empty
 * @return TRUE if stack is empty; FALSE if stack is not empty
 */
static bool stack_isempty(void);
/**
 * @brief a check-function that determines if the stack is full
 * @return TRUE if stack is full; FALSE if stack is not full
 */
static bool stack_isfull(void);
/**
 * @brief calculates the given calculation line
 * @param the line that has to be calculated in postfix notation
 * @return calculated value based on given calculation line
 */
static double calculate(char* calculation);
/**
 * @brief frees the memory from all allocated ressources
 */
static void free_ressources(void);

/****************************************
Error routines
****************************************/
/**
 * @brief prints usage of the program
 */
static void usage(void);
/**
 * @brief terminates the program on program error
 * @param errmessage An errormessage that says what went wrong
 */ 
static void bailout(const char *errmessage);
/**
 * @brief prints error messages
 * @param errmessage
 */
static void printerror(const char* errmessage);
/**************************************
Functionpointer and operations
**************************************/
double (*operation) (double, double);

double add( double a, double b){ return a + b; }
double sub( double a, double b){ return a - b; }
double mul( double a, double b){ return a * b; }
double divide( double a, double b){
  if( b == 0 ){ bailout("divide Division by zero"); }
  return a / b;
}
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
  bool outmode[] = {FALSE, FALSE};

  double result = 0.0;
 
  bool fileread = FALSE;   
  char *in_line = 0;  
  char *res = 0;

  /*****Usage test****/
  if(argc > 0){ prgname = argv[0]; }

  while(( c = getopt( argc,argv,"ai") ) != -1 ){
    switch( c ){
    case 'a':
      outmode[0] = TRUE; break;
    case 'i':
      outmode[1] = TRUE; break;
    case '?': usage(); break;
    default: assert(0);
    }

  } 

  if( create_stack(BFSIZE) == FALSE ){ bailout("main Stack initialization failed"); }
  /******Processing inputs*******/

  if(optind < argc){ /* reading from file(s) */ 
    if( (src_file = fopen(argv[optind],"r")) == NULL ){
      /*file not found*/
      bailout("main No such file"); 
    }

    optind++;
    fileread = TRUE; 
  } else { src_file = stdin; }

  in_line = (char*)malloc(BFSIZE * sizeof(char));

  while( (res=fgets(in_line, BFSIZE, src_file)) !=NULL || (optind < argc) ){
   
    /*for fileinput only - loads the next file from arguments*/ 
    if( res == NULL && fileread == TRUE && optind < argc ){

      if( fclose(src_file) == EOF ){ bailout("main Could not close file"); }      
    
      if( (fopen(argv[optind], "r")) == NULL ){
	free(in_line);
	bailout("main No such file");
	
      }
      optind++;
      continue;
    }
    /*calculation of line read*/
    result = calculate(in_line);

    if( outmode[0] == TRUE ){ result = abs(result); }
    
    if( outmode[1] == TRUE ){ printf("%d\n",(int)result); }
    else{ printf("%f\n",result); }

  }

  /**********clearing and exit***************/
  free_ressources();
  exit(EXIT_SUCCESS);
}
/************************************
Function definitions
 ***********************************/
static double calculate(char* calculation){
  double operand1 = 0.0;
  double operand2 = 0.0;
  double result = 0.0;
  char* hline = calculation;
  char* endptr = (char*) 0;

  while( (*hline != '\n') && (*hline != EOF)){
    operation = 0;
    operand1 = strtod(hline, &endptr);
    errno = 0; 
    if( operand1 == HUGE_VAL && errno != 0 ){ bailout("calculate The given value causes an over- or underflow"); }

    if( operand1 == 0 && hline == endptr ){ 
      /* 
       * check for space is needed because endptr stops one char before operator 
       * the loop is for ignoring all whitespaces in front of the operator
       */
      while( *endptr == ' ' || *endptr == '\t' ){endptr++;}
      /* if there were closing whitespaces after last operand there shouldnt be
       * a wrong operation error, so this if statement breaks the loop */
      if( *endptr == '\n' || *endptr == EOF ){ break; }

      switch(*endptr){
      case '+': operation = &add;break;
      case '-': operation = &sub; break;      
      case '*': operation = &mul; break;
      case '/': operation = &divide; break;
      case 's': operand1 = pop(); push(sin(operand1)); break;
      case 'c': operand1 = pop(); push(cos(operand1)); break;      
      default:
	bailout("Not a valid operation"); 
      }

      if( operation != 0 ){
	operand2 = pop();
	operand1 = pop();
	/*operation is assigned with one of the binary arithmetic operations*/
	push(operation(operand1, operand2));
      }
      /* moves pointer from operation sign */
      endptr++; 
    }else{ push(operand1); }

    hline = endptr;
  }

  if( stack->actsize > 1 ){ bailout("calculate Too many items on the stack"); }
  result = pop();
  return result;
}

static bool create_stack(const unsigned int size){
  if( size <= 0 ){ return FALSE; }

  stack = (Stack*)malloc(sizeof( Stack ));
  if( stack == NULL ){ return FALSE; }
  
  stack->maxsize = size;
  stack->actsize =0;
  stack->esp=(double*)malloc(sizeof(double)*size);

  return TRUE;
}
static void push(const double value){
  if( stack == 0 ){ bailout("push Stack is not initialized"); }
  if( stack_isfull() ==  TRUE ){ bailout("push Stack is full"); }
  stack->esp[stack->actsize] = value;
  stack->actsize++;
 
}
static double pop(void){
  if( stack == 0 ){ bailout("pop Stack is not initialized"); }
  if( stack_isempty() == TRUE ){ bailout("pop Stack is empty"); }
  stack->actsize--;
  return stack->esp[stack->actsize]; 

}
static void destroy_stack(void){
  if( stack == 0 ){ bailout("destroy_stack Stack is not initialized"); }
  free(stack->esp);
  free(stack);
  stack = (Stack *) 0;
}
static bool stack_isempty(void){
  if( stack == 0 ){ bailout("stack_isempty Stack is not initialized"); }
  if( stack->actsize != 0){ return FALSE; }
  return TRUE;
}
static bool stack_isfull(void){
  if( stack == 0 ){ bailout("stack_isfull Stack is not initialized"); }
  if( stack->actsize < stack->maxsize ){ return FALSE; }
  return TRUE;
}

/************************************
Error routines definitions
 ***********************************/
static void usage(void){
  (void)fprintf(stderr,"Usage: %s [-a] [-i] [file1[file2 ...]]\n",prgname);
  bailout((const char*) 0);
}

static void bailout(const char *errmessage){
  if(errmessage != (const char*) 0){ printerror(errmessage); }

  free_ressources();
  exit(EXIT_FAILURE);
}

static void printerror(const char *errmessage){
  if( errno != 0 ){
    (void) fprintf(stderr, "%s: %s - %s\n", prgname, errmessage, strerror(errno));
  }else{
    (void) fprintf(stderr, "%s: %s\n", prgname, errmessage);
  }
}
static void free_ressources(void){

  if(src_file != (FILE *) 0){

    if( fclose(src_file) == EOF ){
      src_file = (FILE *) 0;
      bailout("free_ressources Cannot close file stream");
    }
    src_file = (FILE *) 0;
  }

  if( stack != (Stack *) 0 ){ destroy_stack(); }
}
