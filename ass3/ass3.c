//-----------------------------------------------------------------------------
// ass3.c
//
// Solitaire
//
// Group: 04 - study assistant Mathias Kahr
//
// Authors: Simon Joham 11814175
// Thomas Köhler 11827322
//
// Latest Changes: 13.12.2018 (by Simon Joham)
//-----------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Return values of the program
typedef enum _ReturnValue_
{
  EVERYTHING_OK = 0,
  INVALID_ARGUMENTS = 1,
  OUT_OF_MEMORY = 2,
  INVALID_FILE = 3
} ReturnValue;

typedef enum _StackType_
{
  PICK_OFF_STACK = 0,
  GAME_STACK_1 = 1,
  GAME_STACK_2 = 2,
  GAME_STACK_3 = 3,
  GAME_STACK_4 = 4,
  DEPOSIT_STACK_1 = 5,
  DEPOSIT_STACK_2 = 6,
  MOVE_STACK = 7
} StackType;

//Stores Card
typedef struct Card_
{
  char color;
  char *value;
  struct Card_ *next;
  struct Card_ *prev;
}Card;

//Stores a card stack and which type of stack it is (e.g. GameStack)
typedef struct CardStack_
{
  struct Card_ *top_card; //the stack of cards
  struct Card_ *bottom_card;
  char *stack_type;
}CardStack;

enum CardValue
{
   A = 1,
   J = 11,
   Q = 12,
   K = 13
};

void playLoop(CardStack **stacks);
char **checkForMoveCommand(char *input);
int checkForCardValue(char *input);
char *getInput();
void initStacks(CardStack **stacks);
void freeStacks(CardStack **stacks);
void renderStacks(CardStack **stacks);
void printRows(CardStack **stacks);
void printPickOffStack(Card *card);
void printOtherStacks(Card *card, int stack);
void *mallocCheck(size_t size);
void copyCard(Card *dest, Card *src);
void freeCard(Card *s);
void addTop(CardStack *stack, char color, char *value);
Card delTop(CardStack *stack);
Card *findCard(CardStack *stack, Card *spec_card);
ReturnValue moveStack(CardStack *dest_stack, CardStack *src_stack, char color, char *value);
void printCard(Card *card);
ReturnValue readCardsFromPath(char *path, CardStack **card_stack);
ReturnValue readCardsFromFile(FILE *file, CardStack **card_stack);
ReturnValue checkCards(char **input, int lines);
ReturnValue addCardsToStacks(char **cards, CardStack **card_stack);
char *getCardValue(char *card);
char *copyString(char *string);
ReturnValue printErrorMessage(ReturnValue return_value);

int main(int argc, char **argv)
{
  if(argc != 2)
  {
    return printErrorMessage(INVALID_ARGUMENTS);
  }

  CardStack **stacks = mallocCheck(sizeof(CardStack *) * 7);
  initStacks(stacks);

  ReturnValue return_value = readCardsFromPath(argv[1], stacks);
  if(return_value == EVERYTHING_OK)
  {
    playLoop(stacks);
  }
  freeStacks(stacks);
  
  return printErrorMessage(return_value);
}

void playLoop(CardStack **stacks)
{
  char *input = NULL;
  int running = 1;
  char **move = NULL;
  renderStacks(stacks);
  do
  {
    printf("esp> ");
    input = getInput();
    for(int ch = 0; ch < strlen(input); ch++)
      input[ch] = tolower(input[ch]);
    if(!strcmp(input, "help\n"))
    {
      printf("possible command:\n");
      printf(" - move <color> <value> to <stacknumber>\n");
      printf(" - help\n");
      printf(" - exit\n");
    }
    else if(!strcmp(input, "exit\n"))
      running = 0;
    else if((move = checkForMoveCommand(input)) != NULL) 
      printf("correct move\n");
    else
      printf("[INFO] Invalid command!\n");
    free(input);
    if(move != NULL)
      free(move);
  } while(running);
}

char **checkForMoveCommand(char *input)
{
  char *move, *col, *val, *to, *dest;
  char **move_command = mallocCheck(sizeof(char *) * 3);
  int return_null = 0;
  if(input != NULL)
    move = strtok(input, " ");
  else
    return_null = 1;
  if(move != NULL && !strcmp(move, "move"))
  {
    col = strtok(NULL, " ");
    if(col != NULL && (!strcmp(col,"r") || !strcmp(col,"b")))
    {
      move_command[0] = col;
      val = strtok(NULL, " ");
      if(checkForCardValue(val))
      {
        move_command[1] = val;
        to = strtok(NULL, " ");
        if(to != NULL && !strcmp(to, "to"))
        {
          dest = strtok(NULL, "\n");
          if(dest != NULL && strlen(dest) == 1 && dest[0] >= '0' && dest[0] <= '6')
          {
            move_command[2] = dest;
          }
          else 
            return_null = 1;
        }
        else 
          return_null = 1;
      }
      else 
        return_null = 1;
    }
    else 
      return_null = 1;
  }
  else 
    return_null = 1;
  if(return_null)
  {
    free(move_command);
    return NULL;
  }
  return move_command;
}

int checkForCardValue(char *input)
{
  if(input == NULL)
    return 0;
  if(strlen(input) <= 2 && strlen(input) > 0)
  {
    if(!strcmp(input, "a") || !strcmp(input, "k") || !strcmp(input, "q") || !strcmp(input, "j"))
      return 1;
    else if(strlen(input) == 1 && isdigit(input[0]) && strcmp(input, "1"))
      return 1;
    else if(!strcmp(input, "10"))
      return 1;
  }
  return 0;
}

char *getInput()
{
  char *input = malloc(sizeof(char) * 20);
  int count = 0;
  int trimmed = 0;
  int realloced = 1;
  int ch = getchar();
  while(ch != '\n' && ch != EOF)
  {
    if(!isspace(ch) || trimmed)
    {
      trimmed = 1;
      if(!isspace(ch))
      {
        input[count] = ch;
        ch = getchar();
      }
      else
      {
        do{
          ch = getchar();
        } while(isspace(ch) && ch != '\n');
      	if(ch == '\n' || ch == EOF)
          break;
        else
        {
          input[count] = ' ';
        }
      }
      if(++count + 2 > sizeof(char) * 20 * realloced)
        input = realloc(input, sizeof(char) * 20 * ++realloced);
    }
    else 
      ch = getchar();
  }
  input[count] = '\n';
  input[count + 1] = '\0';
  return input;
}

void initStacks(CardStack **stacks)
{
  for(int stack = 0; stack < 7; stack++)
  {
    stacks[stack] = mallocCheck(sizeof(CardStack));
    stacks[stack]->top_card = NULL;
    stacks[stack]->bottom_card = NULL;
  }
}

void freeStacks(CardStack **stacks)
{
  for(int stack = 0; stack < 7; stack++)
  {
    Card *next = NULL;
    for(Card *card = stacks[stack]->top_card; card != NULL; card = next)
    {
      next = card->next;
      free(card->value);
      free(card);
    }
    free(stacks[stack]);
  }
  free(stacks);
}

void renderStacks(CardStack **stacks)
{
  printf("0   | 1   | 2   | 3   | 4   | DEP | DEP\n");
  printf("---------------------------------------\n");
  printRows(stacks);
}

void printRows(CardStack **stacks)
{
  Card *pick_off_card = stacks[PICK_OFF_STACK]->bottom_card;
  Card **other_stacks_cards = malloc(sizeof(Card *) * 7);
  for(int row = 0; row < 16; row++)
  {
    printPickOffStack(pick_off_card);
    for(int stack = GAME_STACK_1; stack <= DEPOSIT_STACK_2; stack++)
    {
      if(row == 0)
        other_stacks_cards[stack] = stacks[stack]->bottom_card;
      else
        other_stacks_cards[stack] = other_stacks_cards[stack] == NULL ? NULL : other_stacks_cards[stack]->prev;
      printOtherStacks(other_stacks_cards[stack], stack);
    }
    printf("\n");
    pick_off_card = pick_off_card == NULL ? NULL : pick_off_card->prev;
  }
  free(other_stacks_cards);
}

void printPickOffStack(Card *card)
{
  if(card != NULL)
  {
    if(card->prev != NULL)
      printf("X   ");
    else
    {
      printCard(card);
      strlen(card->value) > 1 ? printf(" ") : printf("  ");
    }
  }
  else
    printf("    ");
}

void printOtherStacks(Card *card, int stack)
{
  printf("| ");
    if(card != NULL)
    {
      printCard(card);
      if(stack != DEPOSIT_STACK_2)
        strlen(card->value) > 1 ? printf(" ") : printf("  ");
      else
        if(strlen(card->value) == 1)
          printf(" ");
    }
    else
      stack == DEPOSIT_STACK_2 ? printf("   ") : printf("    ");
}

void *mallocCheck(size_t size)
{
  void *buffer = malloc(size);
  if(buffer == NULL)
    exit(printErrorMessage(OUT_OF_MEMORY));
  return buffer;
}

void copyCard(Card *dest, Card *src)
{
  dest->color = src->color;
  dest->value = src->value;
}

void freeCard(Card *s)
{
  //So nicht möglich, da wir mit stacks arbeiten
  //free(s->next);
}

//Adds new card to the top
void addTop(CardStack *stack, char color, char *value)
{
  // make new card and copy data to it:
  Card *new_card = mallocCheck(sizeof(Card));
  new_card->color = color;
  new_card->value = value;

  Card *old_top = stack->top_card;
  new_card->next = old_top;    // next points to previous top card
  new_card->prev = NULL;
  if(old_top != NULL)
    old_top->prev = new_card;
  else
    stack->bottom_card = new_card;
  stack->top_card = new_card; // top now points to new card
}

//Deletes top card
Card delTop(CardStack *stack)
{
  Card *old_top = stack->top_card;  // remember the old top card

  Card copy_old_top;
  copyCard(&copy_old_top, old_top);

  old_top->next->prev = NULL;
  stack->top_card = old_top->next;       // move top card down
  if(stack->top_card->next == NULL)
  {
    stack->bottom_card = stack->top_card;
  }
  free(old_top);              // now we can free the old card
  return copy_old_top;                // and return the card we remembered
}


ReturnValue moveStack(CardStack *dest_stack, CardStack *src_stack, char color, char *value)
{
  //Muss noch überprüft werden, ob wirklich eine Kopie des Stapels erzeugt wird.
  // Kopiert derzeit nur den Stapel und löscht ihn aus dem alten.
  Card *copy_bottom = mallocCheck(sizeof(Card *));
  copy_bottom = src_stack->bottom_card;

  while(copy_bottom->prev != NULL) //finds the card which prev card is the start of the moving stack
  {
    if(copy_bottom->prev->value == value && copy_bottom->prev->color == color)
    {
      break;
    }
    copy_bottom = copy_bottom->prev;
  }

  CardStack *move_stack = mallocCheck(sizeof(CardStack *)); //creates a stack which will hold the moving stack
  move_stack->stack_type = MOVE_STACK;
  move_stack->bottom_card = copy_bottom->prev;
  move_stack->bottom_card->next = NULL;
  move_stack->top_card = src_stack->top_card;

  Card *copy_top = mallocCheck(sizeof(Card *));
  copy_top = move_stack->top_card;
  while(copy_top->next != NULL)
  {
    if(copy_top->next->value == value && copy_top->next->color == color)
    {
      break;
    }
    copy_top = copy_top->next;
  }
  if(copy_top->next->next != NULL)
  {
    copy_top->next->next = NULL;
  }

  copy_bottom->prev = NULL;
  src_stack->top_card = copy_bottom;
  freeStacks(&move_stack);
  free(copy_bottom);
  free(copy_top);

  return EVERYTHING_OK;
}

//Searches for a specific card
Card *findCard(CardStack *stack, Card *spec_card)
{
   Card *old_top = stack->top_card;
   while(old_top->next != NULL)
   {
     if(old_top->color == spec_card->color && old_top->value == spec_card->value )
     {
       //if a matching card is found, the whole stack starting with the specific card will be returned
       copyCard(spec_card, old_top);
       return spec_card;
     }
     old_top = old_top->next; //move one card down
   }

  return NULL;
}

void printCard(Card *card)
{
  if(card->color != '\0' && card->value != NULL)
    printf("%c%s", card->color, card->value);
}

ReturnValue readCardsFromPath(char *path, CardStack **card_stack)
{
  FILE *file = fopen(path, "r");
  if(file == NULL)
  {
    return INVALID_FILE;
  }
  ReturnValue read_return_value = readCardsFromFile(file, card_stack);

  fclose(file);
  return read_return_value;
}

ReturnValue readCardsFromFile(FILE *file, CardStack **card_stack)
{
  char **cards = mallocCheck(27*8);
  char *line;
  int len, line_counter, ch;
  ReturnValue return_value = EVERYTHING_OK;
  for(line_counter = 0; (ch = getc(file)) != EOF; )
  {
    line = mallocCheck(8);
    line[0] = 0;
    for (len=0; ch != '\n' && ch != EOF; )
    {
      if(len > 7)
      {
        printf("too long");
        return_value = INVALID_FILE;
        free(line);
        break;
      }
      if(!isspace(ch) && !isblank(ch))
        line[len++] = ch;
      ch = getc(file);
    }
    if(strlen(line) != 0)
    {
      line[len] = '\0';
      //printf("%s %d\n", line, (unsigned int)strlen(line));
      cards[line_counter++] = line;
    }
    else
      free(line);
    if(return_value != EVERYTHING_OK)
      break;
  }
  return_value = return_value == EVERYTHING_OK ? checkCards(cards, line_counter) : return_value;
  return_value = return_value == EVERYTHING_OK ? addCardsToStacks(cards, card_stack) : return_value;
  for(int li = 0; li < line_counter; li++)
      free(cards[li]);
  free(cards);
  return return_value;
}

ReturnValue checkCards(char **input, int lines)
{
  if(lines < 26 || lines > 26)
  {
    printf("too short or big");
    return INVALID_FILE;
  }
    
  char **cards = (char *[]){
    "REDA", "RED2", "RED3", "RED4", "RED5", "RED6", "RED7",
    "RED8", "RED9", "RED10", "REDJ", "REDQ", "REDK",
    "BLACKA", "BLACK2", "BLACK3", "BLACK4", "BLACK5", "BLACK6",
    "BLACK7", "BLACK8", "BLACK9", "BLACK10", "BLACKJ", "BLACKQ", "BLACKK"
  };
  char **included_cards = mallocCheck(lines*8);
  int included_counter = 0;
  int line, card;
  for(line = 0; line < lines; line++)
  {
    for(card = 0; card < lines; card++)
    {
      if(!strcmp(input[line], cards[card]))
      {
        for(int included = 0; included < included_counter; included++)
        {
          if(!strcmp(input[line], included_cards[included]))
          {
            printf("not unique %s\n", input[line]);
            free(included_cards);
            return INVALID_FILE;
          }
        }
        included_cards[included_counter++] = input[line];
        continue;
      }
    }
  }
  free(included_cards);
  return included_counter == 26 ? EVERYTHING_OK : INVALID_FILE;
}

ReturnValue addCardsToStacks(char **cards, CardStack **card_stack)
{
  int card = 25;
  for(int round = 0; round < 4; round++)
  {
    for(int stack = GAME_STACK_1 + round; stack <= GAME_STACK_4; stack++)
    {
      addTop(card_stack[stack], cards[card][0], getCardValue(cards[card]));
      card--;
    }
  }
  for(int po_card = 0; po_card <= card; po_card++)
  {
    addTop(card_stack[PICK_OFF_STACK], cards[card][0], getCardValue(cards[card]));
  }
  return EVERYTHING_OK;
}

char *getCardValue(char *card)
{
  switch(card[0])
  {
    case 'R':
      return copyString(&card[3]);
      break;
    case 'B':
      return copyString(&card[5]);
      break;
    default:
      return NULL;
  }
}

char *copyString(char *string)
{
  char *copy = malloc(sizeof(string));
  strcpy(copy, string);
  return copy;
}

//------------------------------------------------------------------------------
///
/// Print message which describes the return value.
///
/// @param return_value type of main return value
///
/// @return parameter return_value
//
ReturnValue printErrorMessage(ReturnValue return_value)
{
  switch(return_value)
  {
    case INVALID_ARGUMENTS:
      printf("[ERR] Usage: ./ass3 [file-name]\n");
      break;
    case OUT_OF_MEMORY:
      printf("[ERR] Out of memory.\n");
      break;
    case INVALID_FILE:
      printf("[ERR] Invalid file!\n");
      break;
    case EVERYTHING_OK:
      //left blank intentionally
      break;
  }
  return return_value;
}