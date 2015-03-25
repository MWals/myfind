/**
 * @file myfind.c
 * @version 1.0
 * @date 2015/02/14
 * 
 * Version 1.0 - 14. Feb 2015
 *
 * Studiengang ICSS SS2015
 * Gruppenmitglieder:
 * ZS, KL, MW
 *
 * Betriebssysteme - Beispiel 1
 * Titel: myfind
 * find:
 * https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=find&s=1
 * Script zum erstellen der Test Ordner Struktur:
 * https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/test-find.sh
 **/

/**
  * -- includes --
  **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
/* http://man7.org/linux/man-pages/man3/errno.3.html */
#include <sys/types.h>
#include <pwd.h>
/* https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=getpwuid&s=3 */
/* https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=getpwnam&s=3 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/* https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=stat&s=2 */
/* https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=lstat&s=2 */
#include <fnmatch.h>
/* https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=fnmatch&s=3 */
#include <time.h>
/* https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=strftime&s=3 */
#include <glob.h>
/* globbing: https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=glob&s=7 */
/* glob(): https://annuminas.technikum-wien.at/cgi-bin/yman2html?m=glob&s=3 */
#include <dirent.h>
/* http://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html */
#include <grp.h>
/* http://pubs.opengroup.org/onlinepubs/7908799/xsh/grp.h.html */
#include <libgen.h>

/**
  * -- defines --
  * \def MAX_LENGTH 
  * \brief defines the maximum length for a string
  * \def ARGUMENTS_PATTERN 
  * \brief defines the number of arguments (with additional variable)
  * \def ARGUMENTS_PATTERN_NAMES
  * \brief lists the valid argument names (with additional variable)
  * \def ARGUMENTS_SOLO
  * \brief defines the number of arguments (without additional variable)
  * \def ARGUMENTS_SOLO_NAMES
  * \brief defines the valid argument names (without additional variable)
  **/

 /*#define debug*/

 #define MAX_LENGTH 6000
 #define ARGUMENTS_PATTERN 5
 #define ARGUMENTS_PATTERN_NAMES {"-user", "-name", "-type", "-path", "-group"}
 #define ARGUMENTS_SOLO 4
 #define ARGUMENTS_SOLO_NAMES {"-print", "-ls", "-nouser", "-nogroup"}

/**
  * -- structures --
  * \struct flag 
  * \brief defines a struct for the arguments given by the user
  **/


typedef struct flag
{
	char argument[MAX_LENGTH];
	char pattern[255];
	int count;
} flag;

/**
  * -- enumerations --
  **/

/**
  * -- global variables --
  **/

const char com_arg_pattern[ARGUMENTS_PATTERN][10] = ARGUMENTS_PATTERN_NAMES;
const char com_arg_solo[ARGUMENTS_SOLO][10] = ARGUMENTS_SOLO_NAMES;
char * prog_name;
char * filename;
int do_recursive = 1;


/**
  * \class arg_check 
  * \brief this function checks the arguments provided by the user
  * \class do_file
  * \brief this function goes through all files, starting from the given dir_name
  * \class output
  * \brief this function generates the output based on the found files
  * \class print_error
  * \brief this function generates a proper output of error messages
  * \class main
  * \brief This is the main function of myfind
  **/

flag * arg_check(int arg_c, const char ** arg_list, int * flag_arg_c, char ** search_paths, int * path_c);
void do_file(const char * dir_name, int * flag_arg_c, flag * flags, int called_from_main);
void output(char * path, char * name, int flag_arg_c, flag * flags, unsigned char filetype);
void print_error(int e_code, const char * argument, int linenumber);
int check(const char * path, flag * flags);
int get_userid(const char * path);
char * get_username(const char * path);
char * get_type(const char * path);
int count_alphanumeric(char * string);

/**
 * \class main
  * The main function generates the parameters for the other functions and calls them if the arguments given by the
  * user are correct.
  **/

int main(int argc, const char * argv[])
{
	flag * flags = NULL; 
	char * search_paths[8];
	int flag_arg_c = 0;
	int path_c = 0;
	int i = 0, j = 0;
	char * buf;
	DIR *dir;
	filename = NULL;
	
	if((prog_name = malloc((strlen(argv[0]) + 1) * sizeof(char *))) == NULL)
	{
		print_error(2, NULL,__LINE__);
		exit(1);
    }
	if((buf = malloc((strlen(argv[0]) + 1) * sizeof(char *))) == NULL)
	{
		print_error(2, NULL,__LINE__);
		exit(1);
    }

	strcpy(buf, argv[0]);							/* strip file name from path */
	prog_name = basename(buf);

	flags = arg_check(argc, argv, &flag_arg_c, search_paths, &path_c);

   #ifdef debug
	for(i = 0; i < flag_arg_c; i++)
	{
        printf("argument %d: %s\npattern %d: %s\ncount: %d\n", i, flags[i].argument, i, flags[i].pattern, flags[i].count);
	}
	printf("path main: %s\n", search_paths[0]);
	printf("path_c main: %d\n", path_c);
    #endif

	for(i = 0; i < path_c; i++)
	{
		if((dir = opendir(search_paths[i]))== NULL)				/* check if a filename with full path is given, */
		{	 												/* if so cut out the path and check if it is valid */
			if(!(filename = malloc((strlen(search_paths[i]) + 1) * sizeof(char))))
			{
				print_error(2, NULL,__LINE__);
				exit(-1);
			}
			strcpy(filename, (strrchr(search_paths[i], '/') + 1));
			do_recursive = 0;

			for(j = strlen(search_paths[i]); j > 0; j--)
			{
				if(search_paths[i][j] == '/')
				{
					search_paths[i][j] = '\0';
					break;
				}
			}
			if((dir = opendir(search_paths[i]))== NULL)
			{
				print_error(3, search_paths[i],__LINE__);
				exit(1);
			}
		do_file(search_paths[i], &flag_arg_c, flags, 0);
		}
		else
		{
			do_file(search_paths[i], &flag_arg_c, flags, 1);
		}
		free(search_paths[i]);
	}


	free(flags);
	free(buf);

	exit(0);
}

/**
  * \class arg_check
  * The arg_check function checks each argument. The valid arguments are added to a char array to prepare them for further processing.
  *\param arg_c Argument count
  * \param arg_list List of arguments
  * \param flag_arg_c Count of argument flags
  * \param search_paths List of search paths
  * \param path_c Count of search paths
  **/

flag * arg_check(int arg_c, const char ** arg_list, int * flag_arg_c, char ** search_paths, int * path_c)
{
        int i = 0, j = 0, fail = 0, arg_loop = 0, sp = 0;
        flag * f_flags = NULL;

		if(!(f_flags = malloc(arg_c * sizeof(flag *))))
        {
            print_error(2, NULL,__LINE__);
            exit(-1);
        }
		if(arg_c == 1)	/* if there are no arguments, search in '.'*/
		{
            if((search_paths[sp] = malloc(strlen(*search_paths) * sizeof(char *))) == NULL)
			{
				print_error(2, NULL,__LINE__);
				exit(-1);
			}
			else
            {
				strcpy(search_paths[sp++], ".");
				*path_c = sp;
				return f_flags;
			}
        }
        else if(arg_c > 1)
        {
            for(i = 1; i < arg_c; i++)
            {
                if(strncmp(arg_list[i], "-", 1) == 0)					/* if a given argument starts with '-'*/
                {
                    if(!(strcmp(arg_list[i], "-user") == 0 ||			/* check if it is valid*/
                         strcmp(arg_list[i], "-name") == 0 ||
                         strcmp(arg_list[i], "-type") == 0 ||
                         strcmp(arg_list[i], "-print") == 0 ||
                         strcmp(arg_list[i], "-ls") == 0 ||
                         strcmp(arg_list[i], "-nouser") == 0 ||
						 strcmp(arg_list[i], "-nogroup") == 0 ||
						 strcmp(arg_list[i], "-group") == 0 ||
                         strcmp(arg_list[i], "-path") == 0))
                    {
                        print_error(1, arg_list[i],__LINE__);					/* if not, print error and exit*/
                        exit(-1);
                    }
					else
                    {
						fail = 1;				/* if an "-" argument is given, no additional search path will be accepted*/
                        for(arg_loop = 0; arg_loop < ARGUMENTS_PATTERN; arg_loop++)		/* go through the possible arguments which require a pattern*/
                        {
							#ifdef debug_arg_check
							printf("checking argument: %s - %s", arg_list[i], com_arg_solo[arg_loop]);
							printf(" == %d\n", strcmp(arg_list[i], com_arg_solo[arg_loop]));
							#endif

                            if(strcmp(arg_list[i], com_arg_pattern[arg_loop]) == 0)
                            {

                                if(arg_list[i+1] == NULL || strncmp(arg_list[i+1], "-", 1) == 0) /* check if there for a pattern */
								{
									print_error(1, arg_list[i+1],__LINE__); 				 /* if there is no pattern, call print_error*/
									exit(-1);									 /* and exit with code 1*/
								} else if (arg_list[i+2] != NULL && strncmp(arg_list[i+2], "-", 1) != 0)
								{
									print_error(1, arg_list[i+2],__LINE__); 				 /* if there is no pattern, call print_error*/
									exit(-1);
								}

								if(strcmp(arg_list[i], com_arg_pattern[2]) == 0) /* if argument is -type*/
								{
									if(!(strcmp(arg_list[i+1], "b") == 0 ||			/* check if pattern is valid*/
										 strcmp(arg_list[i+1], "c") == 0 ||
										 strcmp(arg_list[i+1], "d") == 0 ||
										 strcmp(arg_list[i+1], "p") == 0 ||
										 strcmp(arg_list[i+1], "f") == 0 ||
										 strcmp(arg_list[i+1], "l") == 0 ||
										 strcmp(arg_list[i+1], "s") == 0))
									{
										print_error(1, arg_list[i+1],__LINE__);				/* if not, print error and exit*/
										exit(-1);
									}
								}
								strcpy(f_flags[j].argument, arg_list[i++]);					
								strcpy(f_flags[j].pattern, arg_list[i]);
								f_flags[j++].count = 1;

                            }
                        }

                        for(arg_loop = 0; arg_loop < ARGUMENTS_SOLO; arg_loop++)	/* go through possible action arguments */
                        {
							#ifdef debug_arg_check
							printf("checking argument: %s - %s", arg_list[i], com_arg_solo[arg_loop]);
							printf(" == %d\n", strcmp(arg_list[i], com_arg_solo[arg_loop]));
							#endif

                            if(strcmp(arg_list[i], com_arg_solo[arg_loop]) == 0)
                            {
								if(arg_list[i+1] != NULL && strncmp(arg_list[i+1], "-", 1) != 0) /* check if there is a pattern */
								{
									print_error(1, arg_list[i+1],__LINE__); 				 /* if there is a pattern, call print_error*/
									exit(-1);									 /* and exit with code 1*/
								}
								strcpy(f_flags[j].argument, arg_list[i]);
								f_flags[j++].count = 1;
                            }
                        }
                    }
                }
				if(fail == 0 && strncmp(arg_list[i], "-", 1) != 0)         /* pfade können nur am Beginn stehen */
                {
					if((search_paths[sp] = malloc((strlen(arg_list[i]) + 1) * sizeof(char))) == NULL)
					{
						print_error(2, NULL,__LINE__);
						exit(-1);
					}
					else
					{
						strcpy(search_paths[sp], arg_list[i]);				/* copy the pattern so the array*/
						if(search_paths[sp][0] != '/')
						{
						  print_error(6, search_paths[sp],__LINE__);
						  exit(-1);
						}
						if(search_paths[sp][strlen(search_paths[sp])-1]=='/')
						{
						  search_paths[sp][strlen(search_paths[sp])-1] = '\0';
						}
						#ifdef debug
						printf("arg_check -path: %s\n",search_paths[sp]);
						#endif
						sp++;
					}
                }

				else if(fail == 1 && sp == 0)          /*set path to current directory if not specified by user input*/
				{
					if((search_paths[sp] = malloc(strlen(*search_paths) * sizeof(char *))) == NULL)
					{
						print_error(2, NULL,__LINE__);
						exit(-1);
					}
					else
					{
						strcpy(search_paths[sp++], ".");
					}
				}
            }
        }
        * flag_arg_c = j;       /*set internal argument counter to number of arguments in * flags*/

		* path_c = sp;
        /*printf("\n fail: %d\n", fail);*/
		return f_flags;
}

/**
  * \class print_error
  * The print_error function prints the corresponding error message to the given error_code.
  * \param e_code The current error code
  * \param argument Argument
  * \param linenumber Line in which the error occured
  **/


void print_error(int e_code, const char * argument,int linenumber)
{
	linenumber=linenumber;
	#ifdef debug
	fprintf(stderr, "Error in line %d: ",linenumber);
	#endif
    switch(e_code)
    {
        case 1: fprintf(stderr, "%s: unknown predikate '%s'\n", prog_name, argument); break;
        case 2: fprintf(stderr, "%s: memory allocation error\n", prog_name); break;
        case 3: fprintf(stderr, "%s: %s: %s\n", prog_name, argument, strerror(errno)); break;
		case 4: fprintf(stderr, "%s: `%s' is not the name of a known user\n", prog_name, argument); break;
		case 5: fprintf(stderr, "%s: unable to read link\n", prog_name);break;
		case 6: fprintf(stderr, "%s: `%s' no such file or directory\n", prog_name, argument); break;
		default:fprintf(stderr, "%s: general fault\n", prog_name); break;
    }
    return;
}

/**
  * \class do_file
  * The do_file function processes the current directory.  
  * Therfor it checks each entry within the given directory if it is a directory or file.
  * Afterwards it calls the output function to trigger the proper output for each function.
  * \param dir_name The current directory
  * \param flag_arg_c Argument flag count
  * \param flags Flags
  * \param called_from_main Defines if function is calles from main or another class
  **/

 void do_file(const char * dir_name, int * flag_arg_c, flag * flags, int called_from_main)
 {
    struct dirent *dirp;
    char path[MAX_LENGTH];
    char temp_path[MAX_LENGTH];
    DIR *dir;

    strcpy(path, dir_name);
    strcpy(temp_path, dir_name);

	/* output . of root search directory */
		if((dir = opendir(dir_name))== NULL)
		{
			if(do_recursive != 0) print_error(3, path,__LINE__);
			return;
		}

		while((dirp=readdir(dir))!=NULL)
		{
			if((strcmp(".", dirp->d_name) == 0) && called_from_main)
			{
				printf("output . = %s\n", dirp->d_name);
				called_from_main = 0;
				output(path, dirp->d_name, * flag_arg_c, flags, dirp->d_type);
			}
		}

		if(closedir(dir)<0)
		{
			print_error(3, strerror(errno),__LINE__);
		}

	/* output of every other files*/
		if((dir = opendir(dir_name))== NULL)
		{
			if(do_recursive == 1) print_error(3, path,__LINE__);
			return;
		}

		while((dirp=readdir(dir))!=NULL)
		{
			if((strcmp("..", dirp->d_name) != 0) && (strcmp(".", dirp->d_name) != 0))
			{				
				if(dirp->d_type == DT_DIR)
				{
					if(filename == NULL) 
					{ 
						output(path, dirp->d_name, * flag_arg_c, flags, dirp->d_type);
						strcat(temp_path,"/");
						strcat(temp_path,dirp->d_name);
						if(do_recursive != 0) do_file(temp_path, flag_arg_c, flags,0);
						strcpy(temp_path, path);   /*reset path*/
					}
					else
					{
						if(strcmp(filename, dirp->d_name) == 0) 
						{
							output(path, dirp->d_name, * flag_arg_c, flags, dirp->d_type);
						}
					}
				}
				else
				{
					if(filename == NULL) 
					{ 
						output(path, dirp->d_name, * flag_arg_c, flags, dirp->d_type);
					}
					else
					{
						if(strcmp(filename, dirp->d_name) == 0) 
						{
							output(path, dirp->d_name, * flag_arg_c, flags, dirp->d_type);
						}
					}
				}
			}
		}
		if(closedir(dir)<0)
		{
			print_error(3, strerror(errno),__LINE__);
		}

    return;
 }

/**
  * \class output
  * The output function generates the output. It compares the given arguments to the expected arguments (based on the user input) 
  * and prints them if them if they are matching.
  * Additionally it provides the proper format for all output
  * \param path Path
  * \param name Name
  * \param flags Flags
  * \param filetype Filetype
  * 
  * 
  **/

void output(char * path, char * name, int flag_arg_c, flag * flags, unsigned char filetype)
{
	int showoutput = 1;
	int i = 0;
	char *fullpath;
	char *buf;
	int last_argument = 0;
	int contains_action_arg = 0;


	if((fullpath = malloc(sizeof(char)*(strlen(path)+strlen(name)+2))) == NULL)
	{
		print_error(2 , NULL,__LINE__);
	}
	strcpy(fullpath, path);

	if(strcmp(name, "."))
	{
		strcat(fullpath, "/");
		strcat(fullpath, name);
	}

    while(i < flag_arg_c)
	{
		for(i = last_argument; i < flag_arg_c; i++)
		{
			if (strcmp(flags[i].argument, com_arg_solo[0])==0) /*if argument is print*/
			{
				last_argument = i+1;
				break;
			}
			if (strcmp(flags[i].argument, com_arg_solo[1])==0) /*if argument is ls*/
			{
				last_argument = i+1;
				break;
			}
			if (strcmp(flags[i].argument, com_arg_pattern[1])==0) /*if argument is name*/
			{
				if (fnmatch(flags[i].pattern,name,0))	showoutput = 0;
			}

			if (strcmp(flags[i].argument, com_arg_pattern[3])==0) /*if argument is path*/
			{
				if (fnmatch(flags[i].pattern,fullpath,FNM_NOESCAPE))	showoutput = 0;
			}

			if (strcmp(flags[i].argument, com_arg_pattern[2])==0) /*if argument is type*/
			{

				switch(flags[i].pattern[0])
				{
				  case ('b'): {
								if((filetype == DT_BLK) == 0) showoutput = 0;
								break;
							}
				  case ('c'): {
								if((filetype == DT_CHR) == 0) showoutput = 0;
								break;
							}
				  case ('d'): {
								if((filetype == DT_DIR) == 0) showoutput = 0;
								break;
							}
				  case ('p'): {
								if((filetype == DT_FIFO) == 0) showoutput = 0;
								break;
							}
				  case ('f'): {
								if((filetype == DT_REG) == 0) showoutput = 0;
								break;
							}
				  case ('l'): {
								if((filetype == DT_LNK) == 0) showoutput = 0;
								break;
							}
				  case ('s'): {
								if((filetype == DT_SOCK) == 0) showoutput = 0;
								break;
							}
				  default: break;
				}
			}

			if (strcmp(flags[i].argument, com_arg_pattern[0])==0) /*if argument is user*/
			{
				static int user_arg_count;														/* not needed if we compare it to the original find */
				struct stat sb;
				struct passwd *pwdstructinput;
				stat(fullpath,&sb);
				
				
				pwdstructinput = getpwnam(flags[i].pattern); 									/*search for username*/
				if(pwdstructinput == NULL)
				{
					pwdstructinput = getpwuid(*flags[i].pattern);								/*search for uid, if no suitable username was found*/
				}
				
				if(pwdstructinput == NULL)														/*neither a username nor a uid was found that suits the pattern*/
				{
					if(count_alphanumeric(flags[i].pattern))
					 {
					   
					  showoutput = 0;
					  if(user_arg_count == 0)													/* not needed if we compare it to the original find */
					  {
					  print_error(4 , flags[i].pattern,__LINE__);
					  exit(1);
					  }
					  else																		/* not needed if we compare it to the original find */
					  {
					   exit(0);
					  }
					 }
					else
					{
					  showoutput = 0;
					  exit(0);
					}
				}

				if((sb.st_uid == pwdstructinput->pw_uid) == 0)
				{
					showoutput = 0;
				}
				user_arg_count++; /* not needed if we compare it to the original find */
			}

			if (strcmp(flags[i].argument, com_arg_solo[2])==0) /*if argument is nouser*/
			{
				struct stat sb;
				struct passwd *pwdstruct;
				stat(fullpath,&sb);
				pwdstruct = getpwuid(sb.st_uid);
				if(pwdstruct != NULL)
				{
					showoutput = 0;
				}
			}

			if (strcmp(flags[i].argument, com_arg_solo[3])==0) /*if argument is nogroup*/
			{
				struct stat sb;
				struct group *grpstruct;

				stat(fullpath,&sb);
				grpstruct = getgrgid(sb.st_gid);
				if(grpstruct != NULL)
				{
					showoutput = 0;
				}
			}

			if (strcmp(flags[i].argument, com_arg_pattern[4])==0) /*if argument is grp*/
			{

				struct stat sb;
				struct group *grpstructinput;
				struct group *grpstructfile;
				stat(fullpath,&sb);

				grpstructinput = getgrnam(flags[i].pattern); 									/*search for grpname*/
				if(grpstructinput == NULL)grpstructinput = getgrgid(*flags[i].pattern);			/*search for grpname, if no suitable grpname was found*/
				if(grpstructinput == NULL)														/*neither a grpname nor a gid was found that suits the pattern*/
				{
					print_error(4 , flags[i].pattern,__LINE__);
					showoutput = 0;
					break;
				}

				grpstructfile = getgrgid(sb.st_gid);
				if((grpstructfile->gr_gid == grpstructinput->gr_gid) == 0)
				{
					showoutput = 0;
				}

			}
		}

		if(showoutput)
		{
				if (strcmp(flags[i].argument, com_arg_solo[0])==0) /*if argument is print*/
				{
					printf("%s\n",fullpath);
					contains_action_arg = 1;
				}

				if (strcmp(flags[i].argument, com_arg_solo[1])==0) /*if argument is ls*/
				{
					struct stat sb;
					char mtime[13];											/*last modificationtime */
					struct tm *tminfo;										/*last modificationtime */
					struct passwd *pwdstruct;	 							/*username */
					struct group *grpstruct;								/*groupname */

					if((lstat(fullpath,&sb)) != 0)							/*file status */
					{
						print_error(3 , "stat()",__LINE__);					/*exception handling */
					}
					/* LS PRINT */
					printf(" %ld",(long)sb.st_ino); 						/*nodenumber */
					printf(" %4ld ",(long)sb.st_blocks/2); 					/*blockcount */

					/* permissions */

					if(filetype == DT_DIR) printf("d");
					else if(filetype == DT_REG) printf("-");
					else if(filetype == DT_CHR) printf("c");
					else if(filetype == DT_BLK) printf("b");
					else if(filetype == DT_FIFO) printf("p");
					else if(filetype == DT_LNK) printf("l");
					else if(filetype == DT_SOCK) printf("s");
					else printf("?");

					printf( (sb.st_mode & S_IRUSR) ? "r" : "-"); 			/* permissions */
					printf( (sb.st_mode & S_IWUSR) ? "w" : "-"); 			/* permissions */
					if (sb.st_mode & S_ISUID)
					{
						printf((sb.st_mode & S_IXUSR) ? "s" : "S");
					}
					else
					{
						printf( (sb.st_mode & S_IXUSR) ? "x" : "-"); 			/* permissions */
					}
					printf( (sb.st_mode & S_IRGRP) ? "r" : "-"); 			/* permissions */
					printf( (sb.st_mode & S_IWGRP) ? "w" : "-"); 			/* permissions */
					
					/* Sticky bit logic:
					sticky and x 			= s
					sticky and not x 		= S
					not sticky and x 		= x
					not sticky and not x 	= -
					*/
					
					if((sb.st_mode & S_ISGID) && (sb.st_mode & S_IXGRP))								/* stickybit */
					{
						printf("s");
					}
					else if((sb.st_mode & S_ISGID) && ((sb.st_mode & S_IXGRP)==0))								/* stickybit */
					{
						printf("S");
					}
					else if(((sb.st_mode & S_ISGID)==0) && (sb.st_mode & S_IXGRP))								/* stickybit */
					{
						printf("x");
					}
					else													/* stickybit */
					{
						printf("-"); 										/* permissions */
					}
					
					printf( (sb.st_mode & S_IROTH) ? "r" : "-"); 			/* permissions */
					printf( (sb.st_mode & S_IWOTH) ? "w" : "-"); 			/* permissions */
					
					if (sb.st_mode & S_ISVTX)
					{
						printf((sb.st_mode & S_IXOTH) ? "t" : "T");
					}
					else
					{
						printf( (sb.st_mode & S_IXOTH) ? "x" : "-"); 			/* permissions */
					}
					
					printf("%4ld ",(long)sb.st_nlink); 						/*linkcount*/

					if((pwdstruct = getpwuid(sb.st_uid)) == NULL) 			/*username*/
					{
						printf("%-9d",sb.st_uid);							/*if user doesn't exists in passwd print userid*/
					}
					else
					{
					printf("%-9s",pwdstruct->pw_name); 						/*print username*/
					}

					if((grpstruct = getgrgid(sb.st_gid)) == NULL) 			/*group file entry */
					{
						printf("%-9d",sb.st_gid);							/*if group doesn't exists in etc/group print groupid*/
					}
					else
					{
						printf("%-9s",grpstruct->gr_name); 						/*print groupname*/
					}

					printf("%8ld ",(long) sb.st_size);						/*filesize in bytes*/
					tminfo = localtime(&sb.st_mtime);						/*last modificationtime*/
					strftime(mtime,sizeof(mtime),"%b %d %H:%M",tminfo);		/*last modificationtime*/
					if(mtime[4]=='0')mtime[4]=' ';
					printf("%s ",mtime); 									/*last modificationtime*/

					if(filetype == DT_LNK)									/*show link destination*/
					{

						if((buf = malloc(sizeof(char*)*(sb.st_size + 1))) == NULL)
						{
							print_error(2, NULL,__LINE__);
							exit(-1);
						}


						if(readlink(fullpath, buf, sizeof(char*)*(sb.st_size + 1))==-1)
						{
							print_error(2, NULL,__LINE__);
						}

						printf("%s -> %s\n",fullpath,buf);
						free(buf);

					}
					else printf("%s\n",fullpath); 							/*fullpath*/
					contains_action_arg = 1;
				}
		}


	}
	if(showoutput && (contains_action_arg == 0)) /* if no print or ls was given, do print */
		{
			printf("%s\n",fullpath);
		}
	free(fullpath);
}

/**
 * \class count_alphanumeric
 * The count_alphanumeric function checks if the provided string is alphanumeric
 * \param string String to check
 **/
int count_alphanumeric(char * string)
{
	int alphanumeric_counter = 0;
	int i = 0;

	while(string[i]!='\0')
	{

	 if (string[i] <= 57 && string[i] >= 48)
	 {
		 i++;
		 continue;
	 }
	 else alphanumeric_counter++;
	 i++;
	}
	return alphanumeric_counter;
}
/*
 * =================================================================== eof ==
 */


