#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME 64
#define MAX_DOMAIN 32

enum ERR_NUMS {
   ERR_ALLOCATING_MEMORY = 1,
   ERR_NULL_POINTER,
   ERR_STR_BUFF_OVERFLOW,
   ERR_INPUT_FORMAT,
   ERR_INPUT_FORMAT_SIZE_RULE_VIOLATION,
   ERR_INPUT_FORMAT_ILLEGAL_CHAR
};

typedef struct credentialInformation {
   union {
      char *names[2]; /* pointer to [0] fisrt name, [1] surname */
      char *alias;
   }id;
   char* domain;
} credentials;

/*rules for names or alias*/
int
checkName(const char* name)
{
   int minSize = 3;
   int i = 0;

   while (name[i] != '\0') {
      /* we don't need to put @ in the illegal character list here
       * as everything past the at is considered domain */
      if (name[i] == '.' || name[i] == ' ') {
         return ERR_INPUT_FORMAT_ILLEGAL_CHAR;
      }
      i++;
   }

   if (i < minSize) {
      return ERR_INPUT_FORMAT_SIZE_RULE_VIOLATION;
   }

   return 0;
}

/*rules for domain*/
int
checkDomain(const char* name)
{
   int minSize = 3;
   int i = 1;

   if (name[1] != '@') {
      return ERR_INPUT_FORMAT_ILLEGAL_CHAR;
   }
   while (name[i] != '\0') {
      if (name[i] == '@' || name[i] == ' ') {
         return ERR_INPUT_FORMAT_ILLEGAL_CHAR;
      }
      i++;
   }

   if (i < minSize) {
      return ERR_INPUT_FORMAT_SIZE_RULE_VIOLATION;
   }

   return 0;
}

int
ParseMail(char *mail, void *out)
{
   char *atCharPtr = strchr(mail, '@');
   char *dotCharPtr;
   int atPosition, size, check;

   if (!out || ((credentials*)out)->id.names[0] == NULL ||
       ((credentials*)out)->domain == NULL ) {
      fprintf(stderr, "ERR_ALLOCATING_MEMORY\n");
      return ERR_NULL_POINTER;
   } else if (!atCharPtr) {
      fprintf(stderr, "ERR_INPUT_FORMAT\n");
      return ERR_INPUT_FORMAT;
   }

   atPosition = atCharPtr - mail;
   dotCharPtr = strchr(mail, '.');

   if (dotCharPtr && atPosition > dotCharPtr - mail) {
      if (dotCharPtr - mail > MAX_NAME - 1) {
         fprintf(stderr, "ERR_STR_BUFF_OVERFLOW\n");
         return ERR_STR_BUFF_OVERFLOW;
      }

      if ( ((credentials*)out)->id.names[1] == NULL ) {
         ((credentials*)out)->id.names[1] = (char*)malloc(MAX_NAME);
      }

      strcpy(((credentials*)out)->id.names[0], "\0");
      strncat( ((credentials*)out)->id.names[0], mail, dotCharPtr - mail );

      check = checkName( ((credentials*)out)->id.names[0] );
      if (check) {
         fprintf(stderr, "ERR_INPUT_FORMAT\n");
         return check;
      }
      mail = dotCharPtr + 1; /*move pointer past the dot char*/
   } else {
      free( ((credentials*)out)->id.names[1] );
      ((credentials*)out)->id.names[1] = ((credentials*)out)->id.alias;
   }
   /*
    *if there was an '.' character before '@':
    * out content
    *  ["name"] <- id.names[0] / id.alias
    *  [MAX_NAME bytes of memory ] <- id.names[1]
    *  [MAX_DOMAIN bytes of memory] <- domain
    * mail points to
    *      \/
    *  name.surname@domain.com
    *otherwise:
    * out content
    *  [MAX_NAME bytes of memory ] <- id.alias / id.names[0], id.names[1]
    *  [MAX_DOMAIN bytes of memory] <- domain
    * mail points to
    * \/
    *  alias@domain.com
    */
   if (atCharPtr - mail > MAX_NAME - 1) {
      if ( ((credentials*)out)->id.names[0] ==
          ((credentials*)out)->id.names[1] ) {
         /*to make sure that we do not free the same pointer 2 times */
         ((credentials*)out)->id.names[1] = NULL;
      }
      fprintf(stderr, "ERR_STR_BUFF_OVERFLOW\n");
      return ERR_STR_BUFF_OVERFLOW;
   }

   strcpy(((credentials*)out)->id.names[1], "\0");
   strncat( ((credentials*)out)->id.names[1], mail, atCharPtr - mail );

   check = checkName( ((credentials*)out)->id.names[1] );

   if ( ((credentials*)out)->id.names[0] == ((credentials*)out)->id.names[1]) {
      /*to make sure that we do not free the same pointer 2 times */
      ((credentials*)out)->id.names[1] = NULL;
   }

   if (check) {
      fprintf(stderr, "ERR_INPUT_FORMAT\n");
      return check;
   }

   mail = atCharPtr;
   size = strlen(mail);
   if (size > MAX_DOMAIN) {
      fprintf(stderr, "ERR_STR_BUFF_OVERFLOW\n");
      return ERR_STR_BUFF_OVERFLOW;
   }

   strcpy(((credentials*)out)->domain, mail);

   check = checkDomain( ((credentials*)out)->domain );
   if (check) {
      fprintf(stderr, "ERR_INPUT_FORMAT\n");
      return check;
   }

   return 0;
}

void
printCredentials(credentials person)
{
   printf("\nCREDENTIALS:\n");
   if (person.id.names[1] == NULL) {
      printf("Alias: %s\n", person.id.alias);
   } else {
      printf("First Name: %s\n", person.id.names[0]);
      printf("Surname: %s\n", person.id.names[1]);
   }
   printf("Domain: %s\n", person.domain);
   printf("\n");
}


int main(void)
{
   credentials janK;
   int aux;

   janK.id.alias = (char*)malloc(MAX_NAME);
   janK.id.names[1] = (char*)malloc(MAX_NAME);
   janK.domain = (char*)malloc(MAX_DOMAIN);

   if (!janK.id.alias || !janK.domain || !janK.id.names[1]) {
      fprintf(stderr, "ERR_ALLOCATING_MEMORY\n");
      return ERR_ALLOCATING_MEMORY;
   }

   aux = ParseMail("jan.kowalski@intel.com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("adequatly_longer_test_name.adequatly_longer_test_name"
                   "@adequatly_longer_test_name", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }


   aux = ParseMail("jankowalski@intel.com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("jan.kowalski@intel.com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("jankowalski@@.@.@...com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("janko........@@.@.@...com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("@", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }


   aux = ParseMail(".@", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("@", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }


   aux = ParseMail("adequatly_longer_test_name"
                   ".adequatly_longer__test_surname"
                   "@notadequatly_longer_domain.coma_____________________",
                   (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("notadequatly_longer_alias__________________________________"
                   "__________________________________________________________"
                   "@adequatly_longer_domain.com",
                   (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("adequatly_longer_name."
                   "notadequatly_longer_surname________________________________"
                   "___________________________________________________________"
                   "@adequatly_longer_domain.com",
                   (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("jankowalski@intel.com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   aux = ParseMail("jankowa.lski@intel.com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   free(janK.id.names[0]);
   janK.id.names[0] = NULL;
   free(janK.id.names[1]);
   janK.id.names[1] = NULL;
   free(janK.domain);
   janK.domain = NULL;

   aux = ParseMail("jan.kowalski@intel.com", (void*)&janK);
   if (!aux) {
      printCredentials(janK);
   } else {
      printf("RESULT: %d\n\n",aux);
   }

   return 0;
}