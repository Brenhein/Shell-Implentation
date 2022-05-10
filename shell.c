/*
* Brenden Hein
*
* shell.c
*
* This file processes a series of command line arguemnts to replecate a 
* basic command line interpreter
*/

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fstream>
#include <pthread.h>

using namespace std;

// Forward function declarations
int commandProcessor(vector<string> commands);
vector<string> tokenizeText(string inputString);
void CreateThread(string inputString);

// Global variables
int sequence_cnt = 1; // sequence counter for what command you're on
extern char** environ; // a pointer to an array of environment variables
char username[1025] = {'\0'};


/*Help function to display commands*/
void help()
{
	cout << "halt" << endl;
	cout << "help" << endl;
	cout << "date" << endl;
	cout << "env" << endl;
	cout << "path" << endl;
	cout << "cwd" << endl;
	cout << "cd" << endl;
	cout << "set" << endl;
	cout << "import" << endl;
}


/*displays the current date and time*/
void date()
{
  // gets the current time 
  time_t currentTime = time(0);
  char *timeString = ctime(&currentTime);

  // prints the current time
  cout << "The current time is " << timeString;
}


/*prints all the current environment variables*/
void env()
{
	// loops through and prints all environment variables
	for (char** envVar = environ; *envVar; ++envVar)
	{
		cout << *envVar << endl;
	}
}


/*displays the current search path*/
void path()
{
  string pathVar = getenv("PATH");
  istringstream iss(pathVar);
  string path;

  // splits the path variable into individual paths
  while (getline(iss, path, ':'))
  {
    cout << path << endl;
  }
}


/*displays the current working directory the user is in*/
void cwd()
{
  // gets the working directory using system calls
  char cwd[1025] = {'\0'};
  getcwd(cwd, 1024);

  cout << cwd << endl;
}


/*Manages the current working directory, changing it if provided a
 valid directory.  ~ returns the user to the home directory*/
void cd(string path = "~") 
{ 
  int CD_FLAG;

  // defaulting to the home directory
  if (path == "~")
  {
    char pathNew[7] = "/user/";
    strncat(pathNew, username, 6+strlen(username)+1);
    CD_FLAG = chdir(pathNew);
  }

  // If changing to the home directory of a different user
  else if (path[0] == '~' && path.length() > 1) 
  {
    char pathNew[7] = "/user/";
    strncat(pathNew, path.substr(1).c_str(), 6+strlen(username)+1);
    CD_FLAG = chdir(pathNew);
  }
  
  else
  {
		int length = path.size();
		char pathNew[length+1] = {'\0'};
		strncpy(pathNew, path.c_str(), length);
		CD_FLAG = chdir(pathNew);
  }

  // if the directory doesn't exist
  if (CD_FLAG == -1)
  {
    cout << "Could not change directory to " << path << endl;
  }  

  // updates the pwd environment variable
  char cwd[1025] = {'\0'};
  getcwd(cwd, 1024);
  
  setenv("PWD", cwd, 1);
}

/* Sets the user's envirnoment variable, var, creating it if 
it doesn't exist yet*/
void set(string var, string value = "")
{
  // the program is unsetting a variable
  if (value == "")
  {
    // the variable doesn't exist
    if (getenv(var.c_str()) == NULL)
    {
      cout << "'" << var << "' cannot be unset because it doesn't exist" << endl;
    }    
    else
    {
      unsetenv(var.c_str());
    }
  }

  // the program is setting the variable (creating it if non-existent)
  else 
  {
    // if the variable name is illegal
    if (!(isalpha(var[0])))
    {
      cout << "Variable name must begin with a letter" << endl;
    }
    else
    {
      setenv(var.c_str(), value.c_str(), 1);    
    }
  }
}

// reads a file and runs each line of the file as a command
void importFile(string file)
{
	ifstream fileStream;

	// tries to open the filestream
	fileStream.open(file);
	if (fileStream.fail())  // File could not be opened
	{
		cout << "'" << file << "' could not be opened" << endl;
	}
	else  // File exists and can be opened
	{
		// loops through the lines of the file
		string line;
    vector<string> commands;

    // loops through the lines in the file
		while (getline(fileStream, line))
		{
      // tokenizes the text
      commands = tokenizeText(line);
      if (commands.size() > 0)
      {
        cout << line << endl; 
        int RECOGNIZED = commandProcessor(commands);

        // checks if the command was recognized
        if (RECOGNIZED == -1)
        {
          CreateThread(line);
        }
      }
		}

    fileStream.close();
	}
}

// splits the line of input into tokens
vector<string> tokenizeText(string inputString)
{
	vector<string> commands; // the place to store all the commands
	string token; // the initial string to build tokens

	// reads the input and stores the data
	for (size_t i = 0; i < inputString.size(); i++)
	{
		if (inputString[i] != ' ' && inputString[i] != '\t')
    {
  	  token += inputString[i];
    }
    else 
		{
    	if (token.size() != 0)
      {
  	    commands.push_back(token);
        token = "";
      }
    }
  }

  // If the final token has not been added to commands yet
	if (token.size() > 0)
  {
    commands.push_back(token);
  }

	return commands;
}

// processes a command, returning 1 if success or -1 if failed
int commandProcessor(vector<string> commands)
{
  // flag to determine if command was found
  int RECOGNIZED = 1;

	// command is to code is done
	if (commands[0] == "halt")
	{
		if (commands.size() > 1)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
		}
		exit(0);
	}

	// command is to see what commands are avaiable
	else if (commands[0] == "help")
	{
		if (commands.size() > 1)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
		}

		help();
	}

	// command is to get the current date
	else if (commands[0] == "date")
	{
		if (commands.size() > 1)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
		}

		date();
	}

	// command to get all environment variables
	else if (commands[0] == "env")
	{
		if (commands.size() > 1)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
		}

		env();
	}

	// command is to get the current search path
	else if (commands[0] == "path")
	{
		if (commands.size() > 1)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
		}

		path();
	}

	// command is for getting the current working directory
	else if (commands[0] == "cwd")
	{
		if (commands.size() > 1)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
		}

		cwd();
	}

	// command to change directories
	else if (commands[0] == "cd")
	{
		if (commands.size() > 2)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
			cd(commands[1]);
		}

		// a path or ~ has potentially been supplied
		else if (commands.size() == 2)
		{
			cd(commands[1]);
		}

		// cd will default to the home directory
		else
		{
			cd();
		}
	}

	// the user wants to manage environment variables
	else if (commands[0] == "set")
	{
		// Too many commands
		if (commands.size() > 3)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
			set(commands[1], commands[2]);
		}
		else if (commands.size() == 3) //set environment variable
		{
			set(commands[1], commands[2]);
		}
		else if (commands.size() == 2) //unset environment variable
		{
			set(commands[1]);
		}
		else
		{
			cout << "'set' must be followed by values" << endl;
			cout << "1 arguement - unsets a variable" << endl;
			cout << "2 arguements - sets/creates a variable" << endl;;
		}
	}

	// user wants to read from a file
	else if (commands[0] == "import")
	{
		// Too many commands
		if (commands.size() > 2)
		{
			cout << commands[0] << " : Ignoring non-option arguements" << endl;
      importFile(commands[1]);
		}
    // if the user did not provide a file to import
    else if (commands.size() == 1)
    {
      cout << "'import' takes 1 arguement - a file to read" << endl;
    }
    else
    {
		  importFile(commands[1]);
    }
  }

	// the user entered an invalid command
	else
	{
    RECOGNIZED = -1;
	}

  return RECOGNIZED;
}

/*A function that will be run in a seperate thread to process
any cexternal command (a command not recognized by the shell)*/
void * ExternalCommand(void * arg)
{
  // stores the string as character pointer
  char *command = (char *) arg;

  //calls system to process the external command
  int SYS_FLAG = system(command);

  // checks if command went through
  if (SYS_FLAG == -1)
  {
    cout << "System process creation failed" << endl;
  }

  return nullptr;
}

/*Used to create and run a speerate thread if a command isn't recognized 
 buy the lsit of built in commands*/
void CreateThread(string inputString)
{
  // Makes the input string a char *
  vector<char> inputString_c(inputString.begin(), inputString.end());
  inputString_c.push_back('\0');

  // creates a seperate thread
  pthread_t thread_id;
  int THREAD_CREATE = pthread_create(&thread_id, NULL, ExternalCommand,
          &inputString_c[0]);

  // in case thread failed to be created
  if (THREAD_CREATE != 0)
  {
    cout << "Thread failed to be created" << endl;
  }

  else
  {
    // waits for the seperate thread to finish
    int THREAD_JOIN = pthread_join(thread_id, NULL);

    // in case thread failed to join
    if (THREAD_JOIN != 0)
    {
      cout << "Thread failed to join" << endl;
    }
  }
}

/*Main entry point for function*/
int main(int argc, char* argv[])
{

  getlogin_r(username, 1024); // the username of the person running the program
 
  // If username can't be determined
  if (username == nullptr)
  {
    cout << "User cannot be determined" << endl;
    exit(1);
  }

  vector<string> commands; // will hold the returned list of tokens
  string inputString;

	// main command loop that keeps processing instructions
	while (1)
	{
		// prints the prompt
		cout << "[" << sequence_cnt << " " << username << "] ";

    // takes in input	
		getline(cin, inputString);
	
		// reads a line of commands
		commands = tokenizeText(inputString);

  	// If the user enters no tokens
  	if (commands.size() == 0)
  	{
      inputString = "";
	  	continue;
  	}

    int RECOGNIZED; // flag to see if command was in built in commands list

    // Runs a line of commands
		RECOGNIZED = commandProcessor(commands);

    // If command was not recognized
    if (RECOGNIZED == -1)
    {
      CreateThread(inputString);     
    }

    inputString = "";
		sequence_cnt++; // adds 1 to the sequence counter
	}
}

